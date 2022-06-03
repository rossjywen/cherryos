CC		= x86_64-elf-gcc
CFLAGS	= -g -c -nostdinc -I../include -Wall -m32 -O0

AS		= nasm
ASFLAGS	= -g -f elf32 

OBJS	= 	console.o \
			keyboard.o \
			tty_io.o \
			ll_rw_blk.o \
			hd.o


%.o : %.s
	$(AS) $(ASFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) $^ -o $@


all : $(OBJS)


.PHONY : clean
clean :
	rm -f *.o


