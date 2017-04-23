/** \file
 * Read arbitrary port.
 *
 * This is not as dangerous as wrmem, but you should still be careful!
 * For instance, attempting to read from SMRAM will cause an immediate
 * kernel panic.
 *
 * (c) 2015 Trammell Hudson
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include "DirectHW.h"

static char
printable(
 	uint8_t c
)
{
	if (isprint(c))
		return (char) c;
	return '.';
}


static void
hexdump(
	const uintptr_t base_offset,
	const uint8_t * const buf,
	const size_t len
)
{
	const size_t width = 16;

	for (size_t offset = 0 ; offset < len ; offset += width)
	{
		printf("%08"PRIxPTR":", base_offset + offset);
		for (size_t i = 0 ; i < width ; i++)
		{
			if (i + offset < len)
				printf(" %02x", buf[offset+i]);
			else
				printf("   ");
		}

		printf("  ");

		for (size_t i = 0 ; i < width ; i++)
		{
			if (i + offset < len)
				printf("%c", printable(buf[offset+i]));
			else
				printf(" ");
		}

		printf("\n");
	}
}


enum PortMode {
    kPortMode_Byte,
    kPortMode_Word,
    kPortMode_Dword,
};


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

    int mode = kPortMode_Byte;
	if (argc != 3 && argc != 4)
	{
		fprintf(stderr, "Usage: %s [-b|-w|-d] port\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "-b") == 0)
	{
        mode = kPortMode_Byte;
		argv++;
	} else if (strcmp(argv[1], "-w") == 0)
	{
        mode = kPortMode_Word;
		argv++;
	} else if (strcmp(argv[1], "-d") == 0)
	{
        mode = kPortMode_Dword;
		argv++;
	}
	const uintptr_t addr = strtoul(argv[1], NULL, 0);
    if (mode == kPortMode_Byte) {
        unsigned char value;
        if (darwin_ioread(addr, (unsigned char *)&value, 1)) {
            printf("read error\n");
            return EXIT_FAILURE;
        }
        printf("value = 0x%02x\n", value);
    } else if (mode == kPortMode_Word) {
        unsigned short value;
        if (darwin_ioread(addr, (unsigned char *)&value, 2)) {
            printf("read error\n");
            return EXIT_FAILURE;
        }
        printf("value = 0x%04x\n", value);
    } else if (mode == kPortMode_Dword) {
        unsigned int value;
        if (darwin_ioread(addr, (unsigned char *)&value, 4)) {
            printf("read error\n");
            return EXIT_FAILURE;
        }
        printf("value = 0x%08x\n", value);
    }
	return EXIT_SUCCESS;
}
