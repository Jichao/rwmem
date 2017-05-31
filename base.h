#pragma once

#include <string>
#include <stdint.h>

std::string int2bits(unsigned int value);
std::string int_to_bool_str(int value);
std::string uint64_to_readable_mem(uint64_t value);
uint32_t getbits(uint32_t value, int startBit, int endBit);
void quad_memcpy( uint32_t * const out, const uint32_t * const in, size_t len);
