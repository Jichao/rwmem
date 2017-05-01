#include <iostream>
#include <bitset>
#include <string>
#include <cpuid.h>

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

int main(int argc, const char * argv[]) {
    unsigned int eax,ebx,ecx,edx;
    unsigned long addr = strtoul(argv[1], NULL, 0);
    __get_cpuid((int)addr, &eax, &ebx, &ecx, &edx);
    std::cout << "cpuid for " << addr << ":\n";
    std::cout << "edx: ";
    std::cout << int2bits(edx) << std::endl;
    std::cout << "ecx: ";
    std::cout << int2bits(ecx) << std::endl;
    std::cout << "ebx: ";
    std::cout << int2bits(ebx) << std::endl;
    std::cout << "eax: ";
    std::cout << int2bits(eax) << std::endl;
    return 0;
}
