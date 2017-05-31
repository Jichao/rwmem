#include <iostream>
#include <bitset>
#include <string>
#include <cpuid.h>
extern "C"
{
#include "DirectHW.h"
}

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

int main(int argc, const char * argv[])
{
    if (iopl(0) < 0) {
        perror("iopl");
        return -1;
    }

    uint32_t cpudata[4];
    uint32_t core = 0, eax = 0, ecx = 0;

    if (argc < 3) {
        perror("usage: cpuid core eax [ecx]");
        return -1;
    }

    core = (uint32_t)strtoul(argv[1], NULL, 0);
    eax = (uint32_t)strtoul(argv[2], NULL, 0);
    if (argc >= 4) {
        ecx = (uint32_t)strtoul(argv[3], NULL, 0);
    }

    logical_cpu_select(core);
    rdcpuid(eax, ecx, cpudata);

    printf("cpuid for core: %d eax: %d ecx: %d\n", core, eax, ecx);
    std::cout << "eax: ";
    std::cout << int2bits(cpudata[0]) << "(" << cpudata[0] << ")" << std::endl;
    std::cout << "ebx: ";
    std::cout << int2bits(cpudata[1]) << "(" << cpudata[1] << ")" << std::endl;
    std::cout << "ecx: ";
    std::cout << int2bits(cpudata[2]) << "(" << cpudata[2] << ")" << std::endl;
    std::cout << "edx: ";
    std::cout << int2bits(cpudata[3]) << "(" << cpudata[3] << ")" << std::endl;
    return 0;
}
