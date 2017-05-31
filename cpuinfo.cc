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
#include <vector>
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

static int cpuid(int core, int leaf, int subleaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
{
    uint32_t cpudata[4];
    logical_cpu_select(core);
    rdcpuid(leaf, subleaf, cpudata);
    *eax = cpudata[0];
    *ebx = cpudata[1];
    *ecx = cpudata[2];
    *edx = cpudata[3];
    return 0;
}

struct CpuParam {
    int smtShiftWidth;
    int coreplusSMTShiftWidth;
    uint32_t smtMask;
    uint32_t coreMask;
    uint32_t pkgMask;
};

struct CacheParam {
    int level;
	int type;
	uint32_t size;
    uint32_t idMask;
    uint32_t smtMask;
};

enum CpuLevelType {
    kCpuLevelType_Invalid,
    kCpuLevelType_SMT,
    kCpuLevelType_Core,
};

enum CacheType {
    kCacheType_Invalid,
    kCacheType_Data,
    kCacheType_Instruction,
    kCacheType_Unified,
};

int getCpuParam(CpuParam* params) {
    int core = 0;
    uint32_t eax,ebx,ecx,edx;
    cpuid(core, 0, 0, &eax, &ebx, &ecx, &edx);
    if (eax >= 11) {
        uint32_t levelNumber = 0;
        while (true) {
            cpuid(core, 11, levelNumber, &eax, &ebx, &ecx, &edx);
            uint32_t levelType = getbits(ecx, 8, 15);
            uint32_t shiftWidth = getbits(eax, 0, 4);
            if (levelType == kCpuLevelType_Invalid)
                break;
            else if (levelType == kCpuLevelType_SMT) {
                params->smtShiftWidth = shiftWidth;
                params->smtMask = ~(-1 << shiftWidth);
            } else if (levelType == kCpuLevelType_Core) {
                params->coreplusSMTShiftWidth = shiftWidth;
                params->coreMask = ~(-1 << shiftWidth) ^ params->smtMask;
                params->pkgMask = (-1 << shiftWidth);
            }
            levelNumber++;
        }
    }
    return 0;
}

std::string cacheTypeToString(uint32_t type) {
    if (type == kCacheType_Data) {
        return "Data";
    } else if (type == kCacheType_Instruction) {
        return "Instruction";
    } else if (type == kCacheType_Unified) {
        return "Unified";
    }
    return "Invalid";
}

int countToBitCount(uint32_t count)
{
    count = count * 2 - 1;
    for (int i = 31; i >= 0; --i) {
        if (count & (1 << i)) {
            return i;
        }
    }
    return 0;
}

std::vector<CacheParam> enumCache() {
	std::vector<CacheParam> params;
    int core = 0;
    uint32_t eax,ebx,ecx,edx;
    cpuid(core, 0, 0, &eax, &ebx, &ecx, &edx);
    if (eax >= 11) {
        uint32_t index = 0;
        while (true) {
            cpuid(core, 4, index, &eax, &ebx, &ecx, &edx);
            uint32_t level = getbits(eax, 5, 7);
            uint32_t type = getbits(eax, 0, 4);
            if (type == kCacheType_Invalid)
                break;
            uint32_t lineSize = getbits(ebx, 0, 11) + 1;
            uint32_t linePartition = getbits(ebx, 12, 21) + 1;
            uint32_t ways = getbits(ebx, 22, 31) + 1;
            uint32_t sets = ecx + 1;
            bool fullAssociative = (eax & (1 << 9));
            uint32_t logShare = getbits(eax, 14, 25) + 1;
            uint32_t coreShare = getbits(eax, 26, 31) + 1;

			CacheParam param;
			param.level = level;
			param.type = type;
			param.size = lineSize * linePartition * ways *sets;
			param.smtMask = ~(-1 << countToBitCount(logShare));
			param.idMask = ~(-1 << countToBitCount(coreShare));

			params.push_back(param);

            printf("cache level = %u, cache type = %s, linesize = %u, partition= %u, ways = %u, sets = %u full-associative: %s\n",
                level, cacheTypeToString(type).c_str(), lineSize, linePartition, ways, sets, fullAssociative ? "yes" : "no");
            printf("log share max number = %u(%u bit), core share max number = %u(%u bit), total size = %s\n\n",
                logShare, countToBitCount(logShare), coreShare, countToBitCount(coreShare),
                uint64_to_readable_mem(param.size).c_str());
            index++;
        }
    }
	return params;
}

int main(int argc, char ** argv)
{
    if (iopl(0) < 0) {
		perror("iopl");
        return -1;
	}
    enumCache();
    return 0;
}
