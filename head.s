; step 1 set gdtr to appropriate value
; step 2 set idtr to appropriate value and setup idt table
; step 3 test X87 hardware presence todo
; step 4 setup pagging table

extern main
global _start, idt, gdt
section .text

_start:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	lgdt [gdtr_48]
	jmp 0x0008:setup_idt
	; step 1 is done

setup_idt:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	lidt [idtr_48]

	mov ecx, 0
	mov eax, 0x0008_0000 + unknown_interrupt	; descriptor low 8 bytes
	mov ebx, 0x0000_8E00					; descriptor high 8 bytes
do_setup_idt:
	mov [idt+ecx], eax
	mov [idt+ecx+4], ebx
	add ecx, 8
	cmp ecx, 256*8
	jnz do_setup_idt
	jz setup_pagging
	; step 2 is done

setup_pagging:


	push dead_loop	; this is the return address of main()
	jmp main


dead_loop:
	jmp dead_loop


unknown_interrupt:
	iret


section .data
idtr_48:
	dw 256*8	; limit
				; 256 vectors
	dd idt

align 8
gdtr_48:
	dw 64*8+1	; limit
				; support maxmium 64 tasks
				; each task owns a TSS descriptor and a IDT
				; additionally, a null descriptor is nessesity so plus one

	dd gdt		; base



align 8
idt:
	times 256 dq 0	; same as resq 256
					; 256 interrupt vectors every entry occupies 8 bytes
					; but in fact it could be in C language
					; and use extern to locate it

align 8
gdt:
	dq 0				; null descriptor, selector points to it is called 'null descriptor'

						; code segment base=0 limit=0x00FFF G=1 maximum memory size is 16MB
	dw 0x0FFF			; limit[15:0]=0x0FFF 
	dw 0x0000			; base[15:0]=0
	db 0x00				; base[23:16]=0
	db 0b1001_1010		; C=0 R=1 A=0 execute&read, present=1, dpl=0,
	db 0b1100_0000		; G=1 D=1 AVL=0 limit[19:16]=0x00
	db 0x00				; base[31:24]=0

						; data segment base=0 limit=0x00FFF G=1 maximum memory size is 16MB
	dw 0x0FFF			; limit[15:0]=0x0FFF
	dw 0x0000			; base[15:0]=0
	db 0x00				; base[23:16]=0
	db 0b1001_0010		; P=1 DPL=0 E=0 W=1 A=0 up&write
	db 0b1100_0000		; G=1 B=1 AVL=0 limit[19:16]=0x0
	db 0x00				; base[32:24]=0


