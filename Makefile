all: directories rdmem wrmem rdpci rport wport rdmsr rdmtrr cpuid

.PHONY: directories
directories: bin
bin:
	mkdir -p bin

rdmem: rdmem.o DirectHW.o
	$(CC) -o bin/$@ $^ -framework IOKit
wrmem: wrmem.o DirectHW.o
	$(CC) -o bin/$@ $^ -framework IOKit

rdpci: rdpci.o DirectHW.o
	$(CC) -o bin/$@ $^ -framework IOKit

rport: rport.c DirectHW.o
	$(CC) -o bin/$@ $^ -framework IOKit

wport: wport.c DirectHW.o
	$(CC) -o bin/$@ $^ -framework IOKit

rdmsr: rdmsr.cc DirectHW.o
	g++ -o bin/$@ $^ -framework IOKit

rdmtrr: rdmtrr.cc DirectHW.o
	g++ -std=c++11 -o bin/$@ $^ -framework IOKit
cpuid: cpuid.cc
	g++ -std=c++11 -o bin/$@ $^

clean:
	$(RM) *.o .*.d
	$(RM) -rf bin

test: rdmtrr
	sudo ./rdmtrr
CFLAGS = \
	-g \
	-O3 \
	-W \
	-Wall \
	-MMD \
	-MF .$(notdir $@).d

-include .*.d
