#pragma once

#include <string>
#include <stdint.h>

static inline __attribute__((always_inline)) uint8_t read8(const volatile void *addr)
{
	return *((volatile uint8_t *)(addr));
}

static inline __attribute__((always_inline)) uint16_t read16(const volatile void *addr)
{
	return *((volatile uint16_t *)(addr));
}

static inline __attribute__((always_inline)) uint32_t read32(const volatile void *addr)
{
	return *((volatile uint32_t *)(addr));
}

static inline __attribute__((always_inline)) void write8(volatile void *addr, uint8_t value)
{
	*((volatile uint8_t *)(addr)) = value;
}

static inline __attribute__((always_inline)) void write16(volatile void *addr, uint16_t value)
{
	*((volatile uint16_t *)(addr)) = value;
}

static inline __attribute__((always_inline)) void write32(volatile void *addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}


std::string int2bits(unsigned int value);
std::string int_to_bool_str(int value);
std::string uint64_to_readable_mem(uint64_t value);
uint32_t getbits(uint32_t value, int startBit, int endBit);
uint64_t getbits(uint64_t value, int startBit, int endBit);
void quad_memcpy( uint32_t * const out, const uint32_t * const in, size_t len);


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
