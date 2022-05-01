; step 1 set gdtr to appropriate value
; step 2 set idtr to appropriate value and setup idt table
; step 3 test X87 hardware presence todo
; step 4 setup pagging table

extern main
global _start, page_dir, idt, gdt, kernel_stack
section .text

_start:
page_dir:
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
	mov ss, ax
	mov esp, kernel_stack
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

page_dir_padding:
	times 0x1000-($-$$) db 0 	; 0x1000 is the size of page-table
								; amount of entries in page-table is 2^10 = 1024 
								; entry individual size is 4 bytes
								; whole size -> 2^10 * 4 = 4096 = 0x1000
								; the code below this point is going to be flushed to 0
								; and first 4 entries are going to be four 4MB-pages for kernel
	
	times 0x2000-($-$$) db 0	; when jump to C language a stack is necessary
								; I experienced a tidious bug debugging because of lack of this
								; thanks God I found this bug and fix it :)
kernel_stack:					; this stack is used by kernel task
								; length is 0x1000->4k absolutely enough

; 1. test if 4MB page feature is valid
; 2. initialize four 4MB-pages in [page_dir+0] [page_dir+4] [page_dir+8] [page_dir+12]
; 3. jump to main()
setup_pagging:
	mov eax, 0x01	; invoke cpuid to acquire whether 4MB-page feature is valid
	cpuid
	and edx, 0x08	; edx->bit3 indicates the result
	cmp edx, 0x08
	jne dead_loop	; if PSE is not valid jump to dead loop

	cld
	mov ecx, 0x1000/4	; flush memory 0~0x1000 
	mov edi, 0
	mov eax, 0
	rep stosd			; flush data done

	mov eax, 0x00000087				; 1st 4MB-page
	mov dword [page_dir+0], eax
	mov eax, 0x00400087 			; 2nd 4MB-page
	mov dword [page_dir+4], eax
	mov eax, 0x00800087				; 3rd 4MB-page
	mov dword [page_dir+8], eax
	mov eax, 0x00C00087		 		; 4th 4MB-page
	mov dword [page_dir+12], eax

	mov eax, 0
	mov cr3, eax			; guarantee CR0 has effective value before enable pagging

	mov eax, cr4			; this is very significant
	or eax, 0x10			; before using 4MB-page 
	mov cr4, eax			; CR4.bit4 must be 1

	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax			; enable pagging

	push dead_loop	; this is the return address of main()
	jmp main


dead_loop:
	jmp dead_loop


unknown_interrupt:
	iret


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


