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

int
main(
	int argc,
	char ** argv
)
{
    if (iopl(0) < 0)
	{
		perror("iopl");
		return EXIT_FAILURE;
	}

	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s [core] [addr]\n", argv[0]);
		return EXIT_FAILURE;
	}

    int core = 0;
    uintptr_t addr = 0;
    if (argc >= 2) {
        core = (int)strtol(argv[1], NULL, 0);
    }
    if (argc >= 3) {
        addr = strtoul(argv[2], NULL, 0);
    }
    logical_cpu_select(core);

    msr_t result = rdmsr(addr);
    if (result.hi == INVALID_MSR_HI) {
        printf("write error");
        return EXIT_FAILURE;
    }
    printf("msr for 0x%lx \n", addr);
    std::cout << "lo: ";
    std::cout << int2bits(result.lo) << "(0x" << std::hex << result.lo << ")" << std::endl;

    std::cout << "ho: ";
    std::cout << int2bits(result.hi) << "(0x" << std::hex << result.hi << ")" << std::endl;
    return EXIT_SUCCESS;
}
