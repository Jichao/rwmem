all: directories rdmem wrmem rdpci rport wport rdmsr rdmtrr rdcpuid cpuinfo rdapic

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

rdapic: rdapic.cc DirectHW.o base.o
	g++ -std=c++11 -o bin/$@ $^ -framework IOKit

rdcpuid: rdcpuid.cc DirectHW.o
	g++ -std=c++11 -o bin/$@ $^ -framework IOKit

cpuinfo: cpuinfo.cc DirectHW.o base.o
	g++ -std=c++11 -o bin/$@ $^ -framework IOKit

base.o: base.cc base.h
	g++ -std=c++11 -c -o base.o base.cc

clean:
	$(RM) *.o .*.d
	$(RM) -rf bin

test: rdapic
	sudo ./bin/rdapic

CFLAGS = \
	-g \
	-O3 \
	-W \
	-Wall \
	-MMD \
	-MF .$(notdir $@).d

-include .*.d
