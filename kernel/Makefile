CC		= x86_64-elf-gcc
CFLAGS	= -g -c -nostdinc -I../include -Wall -m32 -O0

AS		= nasm
ASFLAGS	= -g -f elf32 

OBJS	= asm.o \
		  traps.o \
		  vsprintf.o \
		  printk.o \
		  sched.o \
		  system_call.o \
		  signal.o \
		  sys.o \
		  fork.o


%.o : %.s
	$(AS) $(ASFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) $^ -o $@


all : $(OBJS)


.PHONY : clean
clean :
	rm -f *.o


