CC		= x86_64-elf-gcc
CFLAGS	= -g -c -nostdinc -Iinclude -Wall -m32 -O0


AS		= nasm
ASFLAGS	= -g -f elf32


LD		= x86_64-elf-ld


OBJS	= head.o \
		  main.o \
		  kernel/traps.o \
		  kernel/asm.o

KERNEL	= kernel_bin kernel_img bootsect_bin setup_bin


%.o : %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o : %.c
	$(CC) $(CFLAGS) $< -o $@

all : disk_img

disk_img : $(KERNEL)
	dd if=bootsect_bin of=fd.img seek=0 bs=512 count=1
	dd if=setup_bin of=fd.img seek=1 bs=512 count=4
	dd if=kernel_bin of=fd.img seek=5 bs=512 count=240

bootsect_bin : bootsect.s
	$(AS) $^ -f bin -o $@

setup_bin : setup.s
	$(AS) $^ -f bin -o $@

kernel_bin : $(OBJS)
	$(LD) -x -s -Ttext 0x00  -m elf_i386 --oformat binary -o $@ $^ 

kernel_img : $(OBJS)
	$(LD) -Ttext 0x00 -m elf_i386 -o $@ $^ 

.PHONY: clean
clean :
	rm -f *.o
	rm -f bootsect_bin
	rm -f setup_bin
	rm -f kernel_bin
	rm -f kernel_img
	(cd kernel; make clean)


kernel/traps.o : kernel/traps.c
	(cd kernel; make)


