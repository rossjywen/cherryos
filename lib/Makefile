CC		= x86_64-elf-gcc
CFLAGS	= -g -c -nostdinc -I../include -Wall -m32 -O0 -fno-pie -fno-stack-protector

AS		= nasm
ASFLAGS	= -g -f elf32 

OBJS	= 	ctype.o \
			string.o \
			errno.o


%.o : %.s
	$(AS) $(ASFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) $^ -o $@


all : $(OBJS)


.PHONY : clean
clean :
	rm -f *.o


