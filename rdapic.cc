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
#include <sstream>
#include "base.h"
extern "C"
{
    #include "DirectHW.h"
}

enum APICMode {
    xapic_mode,
    x2apic_mode
};

static int ia32_apic_base = 0x1b;
static int ia32_apic_id = 0x802;

static int cpuid(int leaf, int subleaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
{
    uint32_t cpudata[4];
    rdcpuid(leaf, subleaf, cpudata);
    *eax = cpudata[0];
    *ebx = cpudata[1];
    *ecx = cpudata[2];
    *edx = cpudata[3];
    return 0;
}

int main(int argc, char ** argv)
{
    if (iopl(0) < 0) {
		perror("iopl");
        return -1;
	}

    int core = 0;
    if (argc == 2) {
        core = std::stoi(argv[1], NULL, 0);
    }

    printf("select core: %d\n", core);
    logical_cpu_select(core);

    //get apic support information from cpuid
    unsigned int eax,ebx,ecx,edx;
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 9))) {
        std::cout << "local apci not present " << std::endl;
        return 0;
    }
    if (ecx & (1 << 21)) {
        std::cout << "x2apic mode supported" << std::endl;
    }

    //get apic mode
    msr_t apic_msr = rdmsr(ia32_apic_base);
    bool xapic = apic_msr.lo & (1 << 11);
    bool x2apic = apic_msr.lo & (1 << 10);
    APICMode mode;
    if (!xapic && !x2apic) {
        std::cout << "apic disabled" << std::endl;
        return 0;
    } else if (!xapic && x2apic) {
        std::cout << "apic mode invalid value" << std::endl;
        return -1;
    } else if (xapic && !x2apic ) {
        std::cout << "apic mode\n";
        mode = xapic_mode;
    } else {
        std::cout << "x2apic mode\n";
        mode = x2apic_mode;
    }

    //output the apic table
    printf("dump apic information for core: %d\n", core);
    cpuid(0x80000008, 0, &eax, &ebx, &ecx, &edx);
    int physical_addr_bit_count = eax & 0xff;
    uint64_t apic_base =
        (((uint64_t)apic_msr.hi << 32) + apic_msr.lo) & ~0xffffllu & ((1llu << physical_addr_bit_count) - 1);
    printf("apic_base addr = 0x%llx\n", apic_base);
    const uint8_t * const map_buf = (uint8_t*)map_physical(apic_base, 0x1000);
    if (!map_buf) {
        perror("map physical memory failed");
        return -1;
    }

    //dump the inital apic id and current apic id
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    printf("initial apic id: %d\n", getbits(ebx, 24, 31));

    uint32_t apic_id;
    readmem32((uint64_t)(map_buf + 0x20), &apic_id);
    printf("current apic id in register: %d\n", getbits(ebx, 24, 31));

    unmap_physical((void*)map_buf, 0x1000);
    return 0;
}
