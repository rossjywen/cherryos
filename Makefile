CC		= x86_64-elf-gcc
CFLAGS	= -g -c -nostdinc -Iinclude -Wall -m32 -O0 -fno-pie -fno-stack-protector


AS		= nasm
ASFLAGS	= -g -f elf32


LD		= x86_64-elf-ld


OBJS	= head.o \
		  main.o \
		  kernel/traps.o \
		  kernel/asm.o \
		  kernel/vsprintf.o \
		  kernel/printk.o \
		  kernel/sched.o \
		  kernel/system_call.o \
		  kernel/signal.o \
		  kernel/sys.o \
		  kernel/fork.o \
		  kernel/exit.o \
		  mm/page.o \
		  mm/memory.o \
		  driver/console.o \
		  driver/keyboard.o \
		  driver/tty_io.o \
		  driver/ll_rw_blk.o \
		  driver/hd.o \
		  fs/buffer.o \
		  lib/ctype.o \
		  lib/string.o \
		  lib/errno.o

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
	(cd mm; make clean)
	(cd driver; make clean)
	(cd fs; make clean)
	(cd lib; make clean)


kernel/traps.o : kernel/traps.c
kernel/asm.o : kernel/asm.s
kernel/vsprintf.o : kernel/vsprintf.c
kernel/printk.o : kernel/printk.c
kernel/sched.o : kernel/sched.c
kernel/system_call.o : kernel/system_call.s
kernel/signal.o : kernel/signal.c
kernel/sys.o : kernel/sys.c
kernel/fork.o : kernel/fork.c
kernel/exit.o : kernel/exit.c
	(cd kernel; make)

mm/page.o : mm/page.s
mm/memory.o : mm/memory.c
	(cd mm; make)

driver/console.o : driver/console.c
driver/keyboard.o : driver/keyboard.s
driver/tty_io.o : driver/tty_io.c
	(cd driver; make)

fs/buffer.o : fs/buffer.c
	(cd fs; make)

lib/ctype.o : lib/ctype.c include/ctype.h
lib/string.o : lib/string.c
lib/errno.o : lib/errno.c
	(cd lib; make)


