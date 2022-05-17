; two senarios cause page-fault
; 	1.page entry P-bit is 0 -> invoke do_no_page()
;	2.page entry access right check -> invoke do_wp_page()

extern do_wp_page, do_no_page
global page_fault

; an error code is pushed onto the stack

page_fault:
	xchg eax, ss:[esp]	; now eax saves the error code 
						; and old eax is saved onto stack
	push ebx
	push ecx
	push edx

	push ds
	push es
	push fs

	mov bx, 0x10
	mov ds, bx
	mov es, bx
	;mov fs, xxx todo	; linux-0.11 给fs赋值0x10指向内核数据段?
	
	mov ebx, cr2

	push ebx	; C 函数第2个参数是cr2
	push eax	; C 函数第1个参数是error code

	test eax, 1	; 根据error code的P位来确定到底是调用 do_no_page()还是do_wp_page()
	jnz wp		; 根据SDM P位为0的时候说明是 non-present page造成的
				;         P位为1的时候是access right造成的
no:
	call do_no_page
	jmp ret_pf

wp:
	call do_wp_page
	jmp ret_pf

ret_pf:
	pop fs
	pop es
	pop ds
	pop edx
	pop ecx
	pop ebx
	pop eax

	iret

