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

enum DeliveryMode {
    kDeliveryMode_Fixed = 0,
    kDeliveryMode_SMI = 2,
    kDeliveryMode_NMI = 4,
    kDeliveryMode_INIT = 5,
    kDeliveryMode_ExtINT = 7
};

enum TriggerMode {
    kTriggerMode_Edge,
    kTriggerMode_Level
};

enum DeliveryStatus {
    kDeliveryStatus_Idle,
    kDeliveryStatus_SendPending
};

enum DesitinationMode {
    kDestinationMode_Physical,
    kDestinationMode_Logical,
};

enum TimerMode {
    kTimerMode_OneShot,
    kTimerMode_Periodic,
    kTimerMode_TSCDeadline
};

struct LVTEntry {
    uint8_t vector;
    DeliveryMode delivery_mode;
    DeliveryStatus delivery_status;
    bool iipp;
    bool remote_irr;
    TriggerMode trigger_mode;
    bool masked;
    TimerMode timer_mode;
};

enum LVTEntryName {
    kLVT_LINT0 = 0x350,
    kLVT_LINT1 = 0x360,
};
static int lvt_names[] = {kLVT_LINT0, kLVT_LINT1};
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

std::string deliveryModeToString(int value) {
    switch (value) {
        case kDeliveryMode_NMI:
        return "NMI";
        case kDeliveryMode_SMI:
        return "SMI";
        case kDeliveryMode_Fixed:
        return "Fixed";
        case kDeliveryMode_INIT:
        return "INIT";
        case kDeliveryMode_ExtINT:
        return "ExtINT";
    }
    return "";
}

std::string deliveryStatusToString(int status) {
    switch (status) {
        case kDeliveryStatus_Idle:
        return "Idle";
        case kDeliveryStatus_SendPending:
        return "Send Pending";
    }
    return "";
}

std::string triggerModeToString(int mode) {
    switch (mode) {
        case kTriggerMode_Edge:
        return "Edge";
        case kTriggerMode_Level:
        return "Level";
    }
    return "";
}

std::string timerModeToString(int mode) {
    switch (mode) {
        case kTimerMode_OneShot:
        return "OneShot";
        case kTimerMode_Periodic:
        return "Periodic";
        case kTimerMode_TSCDeadline:
        return "TSCDeadline";
    }
    return "";
}

std::string destinationModeToString(int mode) {
    switch (mode) {
        case kDestinationMode_Logical:
        return "Logical";
        case kDestinationMode_Physical:
        return "Physical";
    }
    return "";
}

void dump_lvt_table(const uint8_t * const map_buf)
{
    {
        uint32_t lint0_reg = *(uint32_t*)(map_buf + 0x350);
        uint32_t vector = getbits(lint0_reg, 0, 7);
        uint32_t deliveryMode = getbits(lint0_reg, 8, 10);
        uint32_t deliveryStatus = getbits(lint0_reg, 12, 12);
        uint32_t triggerMode = getbits(lint0_reg, 15, 15);
        uint32_t maskMode = getbits(lint0_reg, 16, 16);
        printf("lint0 vector = %u, deliverMode = %s(%d), deliveryStatus = %s(%d), triggerMode = %s(%d) maskMode = %s\n",
        vector,
        deliveryModeToString(deliveryMode).c_str(), deliveryMode,
        deliveryStatusToString(deliveryStatus).c_str(), deliveryStatus,
        triggerModeToString(triggerMode).c_str(), triggerMode,
        maskMode ? "masked" : "not masked");
    }

    {
        uint32_t lint1_reg = *(uint32_t*)(map_buf + 0x360);
        uint32_t vector = getbits(lint1_reg, 0, 7);
        uint32_t deliveryMode = getbits(lint1_reg, 8, 10);
        uint32_t deliveryStatus = getbits(lint1_reg, 12, 12);
        uint32_t triggerMode = getbits(lint1_reg, 15, 15);
        uint32_t maskMode = getbits(lint1_reg, 16, 16);
        printf("lint1 vector = %u, deliverMode = %s(%d), deliveryStatus = %s(%d), triggerMode = %s(%d) maskMode = %s\n",
        vector,
        deliveryModeToString(deliveryMode).c_str(), deliveryMode,
        deliveryStatusToString(deliveryStatus).c_str(), deliveryStatus,
        triggerModeToString(triggerMode).c_str(), triggerMode,
        maskMode ? "masked" : "not masked");
    }


    {
        uint32_t error_reg = *(uint32_t*)(map_buf + 0x370);
        uint32_t vector = getbits(error_reg, 0, 7);
        uint32_t deliveryMode = getbits(error_reg, 8, 10);
        uint32_t deliveryStatus = getbits(error_reg, 12, 12);
        uint32_t maskMode = getbits(error_reg, 16, 16);
        printf("error_reg vector = %u, deliverMode = %s(%d), deliveryStatus = %s(%d), maskMode = %s\n",
        vector,
        deliveryModeToString(deliveryMode).c_str(), deliveryMode,
        deliveryStatusToString(deliveryStatus).c_str(), deliveryStatus,
        maskMode ? "masked" : "not masked");
    }

    {
        uint32_t performance_counter = *(uint32_t*)(map_buf + 0x340);
        uint32_t vector = getbits(performance_counter, 0, 7);
        uint32_t deliveryMode = getbits(performance_counter, 8, 10);
        uint32_t deliveryStatus = getbits(performance_counter, 12, 12);
        uint32_t maskMode = getbits(performance_counter, 16, 16);
        printf("performance_counter vector = %u, deliverMode = %s(%d), deliveryStatus = %s(%d), maskMode = %s\n",
        vector,
        deliveryModeToString(deliveryMode).c_str(), deliveryMode,
        deliveryStatusToString(deliveryStatus).c_str(), deliveryStatus,
        maskMode ? "masked" : "not masked");
    }

    {
        uint32_t timer = *(uint32_t*)(map_buf + 0x320);
        uint32_t vector = getbits(timer, 0, 7);
        uint32_t deliveryStatus = getbits(timer, 12, 12);
        uint32_t maskMode = getbits(timer, 16, 16);
        uint32_t timerMode = getbits(timer, 17, 18);
        printf("timer vector = %u, deliveryStatus = %s(%d), maskMode = %s timerMode = %s(%d)\n",
        vector, deliveryStatusToString(deliveryStatus).c_str(), deliveryStatus,
        maskMode ? "masked" : "not masked",
        timerModeToString(timerMode).c_str(), timerMode);
    }
}

int dump_apic_timer(const uint8_t * const map_buf)
{
    unsigned int eax,ebx,ecx,edx;
    cpuid(6, 0, &eax, &ebx, &ecx, &edx);
    if (eax & (1 << 2)) {
        printf("apic timer run at constant rate\n");
    } else {
        printf("apic timer stop while deep c-state or speedstep\n");
    }
    uint32_t initCount = *(uint32_t*)(map_buf + 0x380);
    uint32_t currCount = *(uint32_t*)(map_buf + 0x390);
    printf("init count = %u, curr count = %u\n", initCount, currCount);
    return 0;
}

int dump_apic_table(msr_t apic_msr, int mode)
{
    unsigned int eax,ebx,ecx,edx;

    //output the apic table
    cpuid(0x80000008, 0, &eax, &ebx, &ecx, &edx);
    int physical_addr_bit_count = eax & 0xff;
    uint64_t apic_value = (((uint64_t)apic_msr.hi << 32) + apic_msr.lo);
    uint64_t apic_base = getbits(apic_value, 12, physical_addr_bit_count) << 12;
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
    if (mode == xapic_mode) {
        apic_id = *(uint32_t*)(map_buf + 0x20);
    } else {
        msr_t apic_id_msr = rdmsr(ia32_apic_id);
        apic_id = apic_id_msr.lo;
    }
    printf("current apic id in register: %d\n", getbits(apic_id, 24, 31));

    uint32_t version_reg;
    version_reg = *(uint32_t*)(map_buf + 0x30);
    printf("apic version: %d max lvt entry: %d support eoi broadcast: %s\n", getbits(version_reg, 0, 7),
    getbits(version_reg, 16, 23), (version_reg & (1 << 24)) ? "true" : "false");

    //dump the lvt talbe
    printf(ANSI_COLOR_BLUE "===lvt table===\n" ANSI_COLOR_RESET);
    dump_lvt_table(map_buf);

    printf(ANSI_COLOR_BLUE "===apic timer information===\n" ANSI_COLOR_RESET);
    dump_apic_timer(map_buf);

    unmap_physical((void*)map_buf, 0x1000);
    return 0;
}

int get_apic_mode(msr_t apic_msr, int* mode)
{
    bool xapic = apic_msr.lo & (1 << 11);
    bool x2apic = apic_msr.lo & (1 << 10);
    if (!xapic && !x2apic) {
        std::cout << "apic disabled" << std::endl;
        return -1;
    } else if (!xapic && x2apic) {
        std::cout << "apic mode invalid value" << std::endl;
        return -1;
    } else if (xapic && !x2apic ) {
        std::cout << "apic mode\n";
        *mode = xapic_mode;
    } else {
        std::cout << "x2apic mode\n";
        *mode = x2apic_mode;
    }
    return 0;
}

int dump_ioapic()
{
    printf(ANSI_COLOR_BLUE "===io apic===\n" ANSI_COLOR_RESET);
    //assume the io apic register address 0xfec00000
    //standard way need to dump it from madt table from acpi
    volatile uint8_t * const map_buf = (uint8_t*)map_physical(0xfec00000, 0x1000);
    if (!map_buf) {
        perror("map physical memory failed");
        return -1;
    }

    write32(map_buf, 0);
    uint32_t id = read32(map_buf + 0x10);
    printf("apic id = %x\n", getbits(id, 24, 27));

    write32(map_buf, 1);
    uint32_t ver = read32(map_buf + 0x10);
    int max_entries = getbits(ver, 16, 23);
    printf("version %d maxiumum redirection entries %d\n",
        getbits(ver, 0, 7), max_entries);

    for (int i = 0; i < max_entries; ++i) {
        write32(map_buf, 0x10 + i);
        uint32_t lo = read32(map_buf + 0x10);
        write32(map_buf, 0x11 + i);
        uint32_t hi = read32(map_buf + 0x10);

        uint32_t vector = getbits(lo, 0, 7);
        uint32_t deliveryMode = getbits(lo, 8, 10);
        uint32_t destinationMode = getbits(lo, 11, 11);
        uint32_t deliveryStatus = getbits(lo, 12, 12);
        uint32_t iipp = getbits(lo, 13, 13);
        uint32_t rirr = getbits(lo, 14, 14);
        uint32_t triggerMode = getbits(lo, 15, 15);

        printf("vector = %d, delivery mode = %s(%u) desitionation mode = %s(%u), trigger mode = %s(%u)",
            vector, deliveryModeToString(deliveryMode).c_str(), deliveryMode,
            destinationModeToString(destinationMode).c_str(), destinationMode,
            triggerModeToString(triggerMode).c_str(), triggerMode);

        uint32_t dest;
        if (destinationMode == kDestinationMode_Logical) {
            dest = getbits(hi, 24, 31);
        } else {
            dest = getbits(hi, 24, 27);
        }
        printf(" destination = %u\n", dest);
    }
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

    msr_t apic_msr = rdmsr(ia32_apic_base);
    printf("apic_msr = %llx\n", (uint64_t)(apic_msr.lo + ((uint64_t)apic_msr.hi << 32)));

    //get apic mode
    int mode;
    if (0 != get_apic_mode(apic_msr, &mode)) {
        return -1;
    }
    dump_apic_table(apic_msr, mode);

    dump_ioapic();

    return 0;
}
