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
		fprintf(stderr, "Usage: %s [-b|-w|-d] port value\n", argv[0]);
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
	const uintptr_t port = strtoul(argv[1], NULL, 0);
    int result = 0;
    if (mode == kPortMode_Byte) {
        uint8_t value = (uint8_t)strtoul(argv[2], NULL, 0);
        printf("port = %lu, value = %d\n", port, value);
        result = outb(value, port);
    } else if (mode == kPortMode_Word) {
        uint16_t value = (uint16_t)strtoul(argv[2], NULL, 0);
        printf("port = %lx, value = %x\n", port, value);
        result = outw(value, port);
    } else if (mode == kPortMode_Dword) {
        uint32_t value = (uint32_t)strtoul(argv[2], NULL, 0);
        result = outl(value, port);
    }
    if (result) {
        printf("write error");
        return EXIT_FAILURE;
    }
	return EXIT_SUCCESS;
}
