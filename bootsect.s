; step 1 move the bootsect.s code itself from 0x7c000-0x7c200 to 0x90000-0x90200
; step 2 load setup.s code which stores in subsequent 4 sectors from floppy disk to 0x90200-0x90a00
; step 3 load the KERNEL code which stores in subsequent todo sectors to 0x10000-todo
; step 4 jump to setup.s code

BOOT_SECT_SEG equ 0x7c0

REDIRECTION_SEG equ 0x9000

SETUP_SEG equ 0x9020
SETUP_SECTOR_NR equ 4

KERNEL_SECTOR_NR equ 240

%define dbg_mes1 "load setup done"
%strlen dbg_mes_len1 dbg_mes1

%define dbg_mes2 "load kernel done"
%strlen dbg_mes_len2 dbg_mes2

section .text
org 0 
bits 16

	jmp BOOT_SECT_SEG:$start1

start1:
	mov ax, cs
	mov ds, ax

	mov ax, REDIRECTION_SEG	
	mov es, ax

	mov si, 0
	mov di, 0

	mov cx, 512
	rep movsb

	jmp REDIRECTION_SEG:$before_load_setup
; step 1 is done


before_load_setup:
	mov ax, cs
	mov ds, ax

	mov ss, ax
	mov sp, 0xff00	; align the start point to cacheline
	mov ax, ax		; setup the stack for further push operation
					; stack top is 0x9ff00 bottom is 0x90a00(end of setup.s code)
					; length is 0xf500(61k)
	
; 读取floppy的bios服务int 0x13参数:
;    AH      02  
;    AL      读取扇区数  
;    CH      磁道[0, 79]  
;    CL      扇区[1, 18]  
;    DH      磁头[0, 1]  
;    DL      驱动器（0x0 ~ 0x7f表示软盘，0x80 ~ 0xff表示硬盘）  
;    ES：BX   缓冲区地址，即数据读到这里  
; 返回值  
;    CF = 0表示操作成功，此时AH=0，AL=传输的扇区数  
;    CF = 1即carry位置位（可用JC表示跳转）表示操作失败，AH=状态代码 	

load_setup:
	mov ax, SETUP_SEG
	mov es, ax
	mov bx, 0

	mov ah, 2
	mov al, SETUP_SECTOR_NR
	mov ch, 0
	mov cl, 2
	mov dh, 0
	mov dl, 0

	int 0x13

	mov cx, dbg_mes_len1+2
	mov bp, mes1
	call print_mes

load_kernel:
	mov ah, 2
	mov al, 1
	mov dl, 0
	mov dh, [curr_head_nr]
	mov ch, [curr_track_nr]
	mov cl, [curr_sect_nr]
	mov es, [curr_es]
	mov bx, 0
	int 0x13	; invoke BIOS serive 
	jc die		; if not success jump to die

	cmp byte [curr_sect_nr], 18		; check if the whole track has been read
	jne	.not_whole_track_yet		; if whole track is read
	cmp byte [curr_head_nr], 1		; check if two heads have been already read
	jne .not_both_heads_yet			; if only one head is read jump to .not_both_head_yet
	mov byte [curr_sect_nr], 1		; else means both head and all tracks have been read
	mov byte [curr_head_nr], 0		; reset the varibles to appropriate value
	add byte [curr_track_nr], 1			
	jmp .continue					; reset done, jump to .continue

.not_both_heads_yet:				; prepare to read another head
	mov byte [curr_sect_nr], 1
	mov byte [curr_head_nr], 1
	jmp .continue

.not_whole_track_yet:
	add byte [curr_sect_nr], 1
	jmp .continue
	
.continue:
	add word [curr_es], 0x20		; 0x20 << 4 = 512
	add byte [curr_loop], 1			; times already looped
	cmp word [curr_loop], KERNEL_SECTOR_NR
	je transfer_to_setup			; if load done jump off
	jb load_kernel					; else continues



transfer_to_setup:
	mov cx, dbg_mes_len2
	mov bp, mes2
	call print_mes
	jmp SETUP_SEG:0

die:
	jmp die




; function: use BIOS int 0x10 utility to display string
; p1: cx -> charactor number
; p2: bp -> pointer points to string 
print_mes:
	push ax
	push bx
	push cx

	mov ax, cs
	mov ds, ax
	mov es, ax

	mov ah, 3
	mov bh, 0
	int 0x10

	pop cx
	
	mov bx, 0x000c
	mov ax, 0x1301
	int 0x10

	pop bx
	pop ax

	ret

curr_sect_nr:
	db 6
curr_track_nr:
	db 0
curr_head_nr:
	db 0
curr_es:
	dw 0x1000
curr_loop:
	dw 0

mes1:
	db dbg_mes1
	db 0x0D, 0x0A, 0x00
mes2:
	db dbg_mes2
	db 0x0D, 0x0A, 0x00


	times 510-($-$$) db 0 
	dw 0xAA55				; the sign of a boot sector


