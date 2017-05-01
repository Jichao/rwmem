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

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s addr\n", argv[0]);
		return EXIT_FAILURE;
	}

	const uintptr_t addr = strtoul(argv[1], NULL, 0);
    msr_t result = rdmsr(addr);
    if (result.hi == INVALID_MSR_HI) {
        printf("write error");
        return EXIT_FAILURE;
    }
    printf("msr for 0x%lx \n", addr);
    std::cout << "lo: ";
    std::cout << int2bits(result.lo) << "(0x" << std::hex << result.lo << ")" << std::endl;

    std::cout << "ho: ";
    std::cout << int2bits(result.hi) << "(0x" << std::hex << result.lo << ")" << std::endl;
    return EXIT_SUCCESS;
}
