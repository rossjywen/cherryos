; 1. read data from keyboard via port
; 2. convert scan code set1 to ascii

global keyboard_interrupt


; 2 interrelated 8255 PPI ports:
; Data Port (Port PA) 0x60
; Control Port (Port PB) 0x61
%define kb_data_port	0x60
%define kb_control_port 0x61

keyboard_interrupt:
	push eax
	push ebx
	push ecx
	push edx
	push ds
	push es

	mov ax, 0x10
	mov ds, ax
	mov es, ax			; switch to kernel address space

	


acknowledge_and_EOI:
	in al, kb_control_port	; acknowledge
	or al, 0x80
	out kb_control_port, al
	and al, 0x7F
	out kb_control_port, al	; acknowledge done

	mov al, 0x20
	out 0x20, al				; send EOI to master-8259

	;push 0
	;call do_tty_interrupt		; call C function
	;add esp, 4

	pop es
	pop ds
	pop edx
	pop ecx
	pop ebx
	pop eax

	iret





normal_table:

alt_table:

ctrl_table:

shift_table:


