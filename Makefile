CC = gcc
AS = nasm
LD = ld

CFLAGS = -g -m32 -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables \
         -nostdlib -nostdinc -Wall -Wextra -I.
ASFLAGS = -felf32
LDFLAGS = -z max-page-size=0x1000 -melf_i386 -T linker.ld

.PHONY: all
all: os.iso
	

loader.o: loader.asm
	$(AS) $(ASFLAGS) $^ -o $@

kmain.o: kmain.c
	$(CC) $(CFLAGS) -c $^ -o $@

kstdio.o: kstdio.c
	$(CC) $(CFLAGS) -c $^ -o $@

pm.o: pm.c
	$(CC) $(CFLAGS) -c $^ -o $@

interrupt.o: interrupt.c
	$(CC) $(CFLAGS) -c $^ -o $@

syscall.o: syscall.c
	$(CC) $(CFLAGS) -c $^ -o $@

io.o: io.c
	$(CC) $(CFLAGS) -c $^ -o $@

process.o: process.c
	$(CC) $(CFLAGS) -c $^ -o $@

driver/8259a.o: driver/8259a.c
	$(CC) $(CFLAGS) -c $^ -o $@

driver/clock.o: driver/clock.c
	$(CC) $(CFLAGS) -c $^ -o $@

stdlib/memory.o: stdlib/memory.c
	$(CC) $(CFLAGS) -c $^ -o $@

kernel.elf: loader.o kmain.o kstdio.o pm.o interrupt.o syscall.o io.o process.o driver/8259a.o driver/clock.o stdlib/memory.o
	$(LD) $(LDFLAGS) $^ -o $@

os.iso: kernel.elf iso/boot/grub/grub.cfg
	cp kernel.elf iso/boot
	grub-mkrescue --fonts=ascii --locales=en_GB --modules= \
	              --product-name=WDOS --product-version=1.0 -o os.iso iso

.PHONY: run
run: os.iso
	qemu-system-x86_64 -s -cdrom os.iso -m 1024

.PHONY: debug
debug: os.iso
	bochs -f bochsrc

.PHONY: gdb
gdb:
	gdb -x gdb_remote

.PHONY: clean
clean:
	-rm *.o
	-rm *.elf
	-rm *.iso
	-rm driver/*.o
	-rm stdlib/*.o
	
