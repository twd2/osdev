CFLAGS = -g -m32 -fno-stack-protector -fno-builtin -nostdlib -nostdinc -Wall -Wextra -I.
ASFLAGS = -felf32
LDFLAGS = -z max-page-size=0x1000 -melf_i386 -T linker.ld

.PHONY: all
all: os.iso
	

loader.o: loader.asm
	nasm $(ASFLAGS) $^ -o $@

rdtsc.o: rdtsc.asm
	nasm $(ASFLAGS) $^ -o $@

kmain.o: kmain.c
	gcc $(CFLAGS) -c $^

screen.o: screen.c
	gcc $(CFLAGS) -c $^

pm.o: pm.c
	gcc $(CFLAGS) -c $^

interrupt.o: interrupt.c
	gcc $(CFLAGS) -c $^

kernel.elf: loader.o kmain.o screen.o rdtsc.o pm.o interrupt.o
	ld $(LDFLAGS) $^ -o $@

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

.PHONY: clean
clean:
	-rm *.o
	-rm *.elf
	-rm *.iso
