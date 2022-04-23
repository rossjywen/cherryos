; step 1 invoke BIOS utilities to detect harware parameters
; step 2 move KERNEL from 0x10000-0x90000 to 0x00000-0x80000 (will cover and ruin BIOS)
; step 3 re-program 8259A chip re-set interrupt vectors
; step 4 transfer to 32bit mode(protected mode) with a temperory GDT & IDT
; step 5 jump to head.s code which is "now" located in 0x00000

SETUP_SEG equ 0x9020
DATA_TABLE_SEG equ 0x9000

MASTER_8259_INIT_ICW1 equ 0x20
MASTER_8259_INIT_ICW2 equ 0x21
MASTER_8259_INIT_ICW3 equ 0x21
MASTER_8259_INIT_ICW4 equ 0x21
MASTER_8259_OPERATE_OCW1 equ 0x21

SLAVE_8259_INIT_ICW1 equ 0xA0
SLAVE_8259_INIT_ICW2 equ 0xA1
SLAVE_8259_INIT_ICW3 equ 0xA1
SLAVE_8259_INIT_ICW4 equ 0xA1
SLAVE_8259_OPERATE_OCW1 equ 0xA1

%define message0 "in setup module"
%strlen message0_len message0


section .text
bits 16

	jmp SETUP_SEG:$start0
start0:
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov sp, 0xFF
	
	mov cx, message0_len+2
	mov bp, mes0
	call print_mes

	mov ax, DATA_TABLE_SEG
	mov ds, ax
	mov es, ax
	; start detector the harware parameters

	mov ah, 0x03
	mov bh, 0
	int 0x10	
	mov [0], dx		; save cursor position to 0x90000

	mov ah, 0x88
	int 0x15
	mov [2], ax		; save external memory size to 0x90002

	mov ah, 0x0f
	int 0x10
	mov [4], bx		; save display page to 0x90004 todo  equivalent to %bh to 0x90005 why save bx?
	mov [6], al		; save video mode to 0x90006
	mov [7], ah		; save number of charactor column to 0x90007

	mov ah, 0x12
	mov bl, 0x10
	int 0x10
	mov [10], bl	; save graphic memory size to 0x9000A 
					; 0x00 -> 64k
					; 0x01 -> 128k
					; 0x02 -> 192k
					; 0x03 -> 256k
	mov [11], bh	; save color setting to 0x9000B
					; 0x00 -> color mode (3Dx address range)
					; 0x01 -> monochrome mode (3Bx address range)

	mov [12], cl	; save switch setting to 0x9000C
	mov [13], ch	; save adapter bits to 0x9000D

	lds si, [4 * 0x41]		; BIOS spec inform that 
	mov ax, DATA_TABLE_SEG	; Software interrupt 0x41 points to the
	mov es, ax				; entry in the table for drive 0
	mov di, 0x80			; intend to copy drive0 table to 0x90080-0x90090
	mov cx, 16		
	rep movsb
	; step 1 is done


start_move_kernel:
	cli			; disable interrupt
	cld			; set direction of movsw
	mov ecx, 0
	mov esi, 0
	mov edi, 0
	mov ax, 0

do_move_kernel:
	mov es, ax
	add ax, 0x1000
	mov ds, ax
	mov cx, 0x8000	; every time move 0x10000 bytes 
	rep movsw		; so it has equivalent effect of add 0x1000 to segment

	cmp ax, 0x8000	; ax=0x8000 means moving 0x80000-0x90000 to 0x70000-0x80000 is done
	jz $end_move_kernel
	jmp $do_move_kernel
	; step 2 is done

end_move_kernel:
	mov ax, cs
	mov ds, ax
	mov es, ax
	
reprogram_8259:
	mov al, 0x11					; set ICW1 (bit4 is fixed at 1)
	out MASTER_8259_INIT_ICW1, al	;			1.SNG=0 indicates there is slave 8259
									;		   	2.ICW4 is needed
	mov al, 0x20					; set ICW2 interrupt vetor from 0x20-0x27
	out MASTER_8259_INIT_ICW2, al
	mov al, 0x04					; set ICW3 to specify which pin is connected to slave
	out MASTER_8259_INIT_ICW3, al
	mov al, 0x01					; set ICW4	1.x86 mode
	out MASTER_8259_INIT_ICW4, al	;			2.normal EOI
									;			3.normal fully nested mode
	mov al, 0xFF
	out MASTER_8259_OPERATE_OCW1, al; set mask reg as 0xFF to mask out all interrupts

	mov al, 0x11					; same as master
	out SLAVE_8259_INIT_ICW1, al
	mov al, 0x28					; set ICW2 interrupt vetor from 0x28-0x2F
	out SLAVE_8259_INIT_ICW2, al
	mov al, 0x02					; set ICW3 to fit SLAVE ID 0x02 to be consistent to master ICW3
	out SLAVE_8259_INIT_ICW3, al	
	mov al, 0x01					; same as master
	out SLAVE_8259_INIT_ICW4, al
	mov al, 0xFF
	out SLAVE_8259_OPERATE_OCW1, al	; set mask reg as 0xFF to mask out all interrupts
	; step 3 is done

transfer_to_32bit:
	mov eax, cr0
	lidt [idtr_48]
	lgdt [gdtr_48]
	or eax, 0x1
	mov cr0, eax	
	jmp 0x0008:0	; jump to head.s
	; step 4 is done, now it is in 32bit mode
	; step 5 is done

	

jump_to_kernel:
	jmp $jump_to_kernel

; function: use BIOS int 0x10 utility to display string
; p1: cx -> charactor number
; p2: bp -> pointer points to string 
print_mes:
	push ax
	push bx
	push cx

	;mov ax, cs
	;mov ds, ax
	;mov es, ax

	mov ah, 3
	mov bh, 0
	int 0x10

	pop cx
	
	mov bx, 0x000C
	mov ax, 0x1301
	int 0x10

	pop bx
	pop ax

	ret

mes0:
	db message0, 0x0D, 0x0A



align 8
gdt:
	dq 0				; null entry, selector points to it is called 'null descriptor'

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


align 8
gdtr_48:				; temperory gdtr 
	dw 2048 			; limit = 2048 
	dd 0x90200 + gdt	; base = ds < 4 + gdt (now it is still in 16bit mode)
						; number of entries = 2048 / 8 = 256
						
idtr_48:				; temperory idtr
	dw 0		 		; base->0 
	dd 0				; limit->0
						; because interrupt is disable
						; so this value is just for setting idtr
						; interrupt will never be triggered

;align 8
;	db 0x12, 0x34, 0x56, 0x78
;	dw 0x1234
;	dd 0x12345678

