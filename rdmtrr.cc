#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <iostream>
#include <bitset>
#include <cpuid.h>
#include <sstream>
extern "C"
{
    #include "DirectHW.h"
}

static int ia32_mtrrcap = 0xfe;
static int ia32_mtrr_physbase0 = 0x200;
static int ia32_mtrr_fix64k_0000 = 0x250;
static int ia32_mtrr_fix16k_80000 = 0x258;
static int ia32_mtrr_fix16k_A0000 = 0x259;
static int ia32_mtrr_fix4k_c0000 = 0x268;
static int ia32_mtrr_fix4k_c8000 = 0x269;
static int ia32_mtrr_fix4k_d0000 = 0x26a;
static int ia32_mtrr_fix4k_d8000 = 0x26b;
static int ia32_mtrr_fix4k_e0000 = 0x26c;
static int ia32_mtrr_fix4k_e8000 = 0x26d;
static int ia32_mtrr_fix4k_f0000 = 0x26e;
static int ia32_mtrr_fix4k_f8000 = 0x26f;
static int ia32_pat = 0x277;
static int ia32_mtrr_def_type = 0x2ff;

enum MemoryType {
    kMemoryType_UC = 0,
    kMemoryType_WC = 1,
    kMemoryType_WT = 4,
    kMemoryType_WP = 5,
    kMemoryType_WB = 6,
    kMemoryType_UNDEF = 2333
};

std::string int2bits(unsigned int value)
{
    std::bitset<32> x(value);
    std::string str =
            x.to_string<char,std::string::traits_type,std::string::allocator_type>();
    int pos = 4;
    while (pos < str.length()) {
        str.insert(pos, ",");
        pos += 5;
    }
    return str;
}

std::string int_to_bool_str(int value)
{
    if (value)
        return "enabled";
    else
        return "disabled";
}

std::string int_to_mtrr_type(int type)
{
    switch (type) {
        case kMemoryType_UC:
            return "UC";
        case kMemoryType_WB:
            return "WB";
        case kMemoryType_WP:
            return "WP";
        case kMemoryType_WC:
            return "WC";
        case kMemoryType_WT:
            return "WT";
        default:
            return "UNDEF";
    }
}

std::string uint64_to_readable_mem(uint64_t value)
{
    if (!value) {
        return "0";
    }
    std::stringstream result;
    auto gb = value / (1 << 30);
    if (gb) {
        result << gb << "GB";
        value -= gb * (1 << 30);
    }
    auto mb = value / (1 << 20);
    if (mb) {
        result << " " << mb << "MB";
        value -= mb * (1 << 20);
    }

    auto kb = value / (1 << 10);
    if (kb) {
        result << " " << kb << "KB";
        value -= kb * (1 << 10);
    }
    return result.str();
}

void output_fix_mtrr(int start_msr, int msr_count, int start_mem, int mem_unit)
{
    int prev_mem_type = kMemoryType_UNDEF;
    int mem_len = 0;
    int mem_type;
    for (int i = start_msr; i < start_msr + msr_count; ++i) {
        msr_t msr_value = rdmsr(i);
        uint64_t value = msr_value.lo + ((uint64_t)msr_value.hi << 32);
        for (int j = 0; j < 8; ++j) {
            mem_type = (value >> (j * 8)) & 0xff;
            if (mem_type != prev_mem_type) {
                if (mem_len == 0) {
                    std::cout << "\t0x" << std::hex << start_mem;
                } else {
                    std::cout << " - 0x" << (start_mem + mem_len) << " ("
                        << std::dec << start_mem / 1024 << "k - " << (start_mem + mem_len) / 1024 << "k ) : "
                        << int_to_mtrr_type(prev_mem_type) << "\n";
                    start_mem += mem_len;
                    mem_len = 0;
                    std::cout << "\t0x" << std::hex << start_mem;
                }
                prev_mem_type = mem_type;
            }
            mem_len += mem_unit;
        }
    }
    if (mem_len) {
        std::cout << " - 0x" << (start_mem + mem_len) << " ("
            << std::dec << start_mem / 1024 << "k - " << (start_mem + mem_len) / 1024 << "k ) : "
            << int_to_mtrr_type(prev_mem_type) << "\n";
    }
}

uint64_t get_var_value(msr_t msr_value, int addr_bits)
{
    uint64_t value = msr_value.lo + ((uint64_t)msr_value.hi << 32);
    return ((value >> 12) & (((uint64_t)1 << addr_bits) - 1)) << 12;
}

void output_var_mtrr(int count, int addr_bits)
{
    for (int i = 0; i < count; ++i) {
        int base_index = ia32_mtrr_physbase0 + (2 * i);
        msr_t base_msr = rdmsr(base_index);

        int range_index = base_index + 1;
        msr_t range_msr = rdmsr(range_index);
        if (!(range_msr.lo & (1 << 11))) {
            continue;
        }

        uint64_t phys_base = get_var_value(base_msr, addr_bits);
        uint64_t phys_mask = get_var_value(range_msr, addr_bits);
        uint64_t phys_end = phys_base + ((uint64_t)1 << addr_bits) - phys_mask;

        std::cout << "\t0x" << std::hex << phys_base
            << " - 0x" << phys_end << " ("
            << uint64_to_readable_mem(phys_base) << " - " << uint64_to_readable_mem(phys_end) << ") : "
            << int_to_mtrr_type(base_msr.lo & 0xff) << "\n";
    }
}

int main(int argc, char ** argv)
{
    if (iopl(0) < 0) {
		perror("iopl");
        return -1;
	}

    //get mtrr support information from cpuid
    unsigned int eax,ebx,ecx,edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 12))) {
        std::cout << "mtrr not supported" << std::endl;
        return 0;
    }

    //dump mtrr cap msr
    msr_t cap_msr = rdmsr(ia32_mtrrcap);
    int variable_mtrr_count = cap_msr.lo & 0xff;
    bool fixed_range_mtrr_enable = cap_msr.lo & 0x100;
    bool wc_flag = cap_msr.lo & (1 << 10);
    bool ssmrr_flag = cap_msr.lo & (1 << 11);
    std::cout << "mtrr cap msr dump:\n";
    std::cout << "\tvariable mtrr count: " << variable_mtrr_count << "\n";
    std::cout << "\tfixed-range mtrr status: " << int_to_bool_str(fixed_range_mtrr_enable) << "\n";
    std::cout << "\twrite combining status: " << int_to_bool_str(wc_flag) << "\n";
    std::cout << "\tsmrr status: " << int_to_bool_str(ssmrr_flag) << "\n";

    //dump the default mtrr type msr
    msr_t def_msr = rdmsr(ia32_mtrr_def_type);
    bool mtrr_enable = def_msr.lo & (1 << 11);
    fixed_range_mtrr_enable ^= def_msr.lo & (1 << 10);
    std::cout << "\nmtrr default msr dump:\n";
    std::cout << "\tdefault memory type: " << int_to_mtrr_type(def_msr.lo & 0xff) << "\n";
    std::cout << "\tmtrr status: " << int_to_bool_str(mtrr_enable) << "\n";
    std::cout << "\tfixed-range mtrr status: " << int_to_bool_str(fixed_range_mtrr_enable) << "\n";
    if (!mtrr_enable) {
        return 0;
    }

    //dump the fixed-range mtrrs
    //  1. 0x0->0x7ffff(0-512KB)
    //  2. 0x80000->0xc0000(512kb-768kb)
    //  3. 0xc0000->0xfffff(768kb-1mb)
    if (fixed_range_mtrr_enable) {
        std::cout << "\nfixed-range mtrrs dump:\n";
        output_fix_mtrr(ia32_mtrr_fix64k_0000, 1, 0x0, 64 * 1024);
        output_fix_mtrr(ia32_mtrr_fix16k_80000, 2, 0x80000,  16 * 1024);
        output_fix_mtrr(ia32_mtrr_fix4k_c0000, 8, 0xc0000, 4 * 1024);
    }

    //dump variable mtrrs
    if (variable_mtrr_count) {
        unsigned int eax,ebx,ecx,edx;
        std::cout << "\nvariable mtrrs dump:\n";
        __get_cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
        int physical_addr_bit_count = eax & 0xff;
        std::cout << "physical address bit count : " << physical_addr_bit_count << "\n";
        output_var_mtrr(variable_mtrr_count, physical_addr_bit_count);
    }
    return 0;
}
