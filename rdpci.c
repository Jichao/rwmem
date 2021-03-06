/** \file
 * Poke the PCI port with a value and read back the data.
 *
 * Mapping defined here: http://wiki.osdev.org/PCI_Express#Enhanced_Configuration_Mechanism
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include "DirectHW.h"

int
port_main(int argc, char** argv);
static const int kAddressPort = 0xcf8;
static const int kDataPort = 0xcfc;

int main(int argc, char** argv)
{
	if (iopl(0) < 0)
	{
		perror("iopl");
		return EXIT_FAILURE;
	}

	return port_main(argc, argv);
}

int
port_main(int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s bus device func\n", argv[0]);
		return EXIT_FAILURE;
	}

	const uint32_t bus = strtoul(argv[1], NULL, 0);
	const uint32_t device = strtoul(argv[2], NULL, 0);
	const uint32_t func = strtoul(argv[3], NULL, 0);
	for (int offset = 0; offset < 64; ++offset) {
		const uint32_t addr =
			((bus & 0xff) << 16) |
			((device & 0x1f) << 11) |
			((func & 0x07) << 8) |
			((offset & 0x7f) << 2) |
			(1 << 31);
		outl(addr, kAddressPort);
		printf("offest: %08x value: %08x\n", offset * 4, inl(kDataPort));
	}
	return 0;
}

int
mem_main(
	int argc,
	char ** argv
)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s bus slot func\n", argv[0]);
		return EXIT_FAILURE;
	}

	const uint32_t bus = strtoul(argv[1], NULL, 0);
	const uint32_t slot = strtoul(argv[2], NULL, 0);
	const uint32_t func = strtoul(argv[3], NULL, 0);
	const uint32_t reg = 0; //strtoul(argv[4], NULL, 16);

	const uint32_t addr = 0xe0000000
		| ((bus  & 0xFF) << 20) // 8 bits
		| ((slot & 0x1F) << 15) // 5 bits
		| ((func & 0x07) << 12) // 3 bits
		| ((reg  & 0xFFC) << 0) // 12 bits, minus bottom 2
		;

	const uintptr_t page_mask = 0xFFF;
	const uintptr_t page_offset = addr & page_mask;
	const uintptr_t map_addr = addr & ~page_mask;
	const size_t map_len = (page_offset + 256 + page_mask) & ~page_mask;

	const uint8_t * const pcibuf = map_physical(map_addr, map_len);
	if (pcibuf == NULL)
	{
		perror("map");
		return EXIT_FAILURE;
	}

	for (unsigned i = 0 ; i < 256 ; i+=4)
	{
		printf("%08x=%08x\n", addr+i, *(uint32_t*)(pcibuf + page_offset + i));
	}

	return EXIT_SUCCESS;
}
