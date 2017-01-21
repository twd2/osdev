CC = gcc
LD = ld

CFLAGS = -g -m32 -fno-stack-protector -fno-asynchronous-unwind-tables \
         -Wall -Wextra -I../
LDFLAGS = -melf_i386 -dynamic-linker /lib/ld-linux.so.2 -lc -lm

.PHONY: all
all: test_buddy.elf test_page_list.elf
	

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

test_buddy.elf: test_buddy.c ../mm/buddy.o ../mm/page_list.o ../stdlib/bits.o
	$(CC) $(CFLAGS) $^ -o $@

test_page_list.elf: test_page_list.c ../mm/page_list.o
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: test
test: test_page_list.elf test_buddy.elf
	./test_page_list.elf
	./test_buddy.elf

.PHONY: clean
clean:
	-rm *.d
	-rm *.o
	-rm *.elf
	-rm *.iso
	-rm ../driver/*.o
	-rm ../mm/*.o
	-rm ../stdlib/*.o
