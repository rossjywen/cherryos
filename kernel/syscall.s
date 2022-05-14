; 这个文件存放的都是用户能访问到的中断/异常 相关代码
; 典型的就是system call的实现 vector 0x80 中断的handler
; 其他还包括比如 8254引发的中断handler
; 和kernel/asm.s不同的地方还包括这里处理完中断之后还需要检查信号量
; 我不打算像linux-0.11那样在汇编层面实现很多功能 而是更多的放在C层面实现

extern do_timer_interrupt, do_signal

global  timer_interrupt

; interrupt/exception without CPL change
; note: if a task was interrupted by 8254 in kernel mode
;       schedule() will not be invoked

; +--------------+
; | 1.old EFLAGS |
; +--------------+
; | 2.old CS     |
; +--------------+
; | 3.old EIP    | <- esp
; +--------------+

; if it is int 0x80 the eax maintains syscall number
; CS (and SS if interrupted not in system call) are automatically switched 
; during access program with entry in IDT

; then other info will be pushed onto the stack as follow
; +--------------+
; | 1.old FS     |
; +--------------+
; | 2.old ES     |
; +--------------+
; | 3.old DS     |
; +--------------+
; | 4.EBP        |
; +--------------+
; | 5.ESI        |
; +--------------+
; | 6.EDI        |
; +--------------+
; | 7.EDX(sc p3) |
; +--------------+
; | 8.EDX(sc p2) |
; +--------------+
; | 9.EBX(sc p1) |
; +--------------+
; |10.EAX(sc nr) |
; +--------------+


; 1. detect signal pending and call do_signal()
; 2.
ret_from_syscall:
	call do_signal

	pop eax
	pop ebx
	pop ecx
	pop edx
	pop edi
	pop esi
	pop ebp
	pop ds
	pop es
	pop fs

	iret



timer_interrupt:
	mov al, 0x20
	out 0x20, al

	push fs
	push es
	push ds
	push ebp
	push esi
	push edi
	push edx
	push ecx
	push ebx
	push eax

	mov ax, 0x10	; 切换到内核态的ds 和es
	mov ds, ax
	mov es, ax
	mov ax, 0x17	; 确保fs指向用户代码的数据段
	mov fs, ax

	call  do_timer_interrupt

	jmp ret_from_syscall

	




