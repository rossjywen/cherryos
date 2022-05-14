; 这个文件存放的都是内核来handle的中断/异常相关的代码
; 在SDM中 vector处于0-15的中断/异常(大多数是异常)

extern do_divide_error_0, do_debug_1, do_nmi_2, do_int3_3, do_overflow_4, do_bounds_5, do_invalid_op_6, do_device_not_available_7, do_double_fault_8, do_invalid_TSS_10, do_segment_not_present_11, do_stack_segment_error_12, do_general_protection_13, do_page_fault_14, do_coprocessor_error_16

global divide_error_0, debug_1, nmi_2, int3_3, overflow_4, bounds_5, invalid_op_6, device_not_available_7, double_fault_8, invalid_TSS_10, segment_not_present_11, stack_segment_error_12, general_protection_13, page_fault_14, coprocessor_error_16

section .text

; interrupt/exception with CPL change
; note: error code is optional due to manual
; the stack layout is as below after entrance of corresponding handler
; +--------------+
; | 1.old SS     |
; +--------------+
; | 2.old ESP    |
; +--------------+
; | 3.old EFLAGS |
; +--------------+
; | 4.old CS     |
; +--------------+
; | 5.old EIP    | <- esp (if without error code)
; +--------------+
; | 6.error code | <- esp (if with error code)
; +--------------+







divide_error_0: ; vector 0, no error code
	push eax					; save eax
	mov eax, do_divide_error_0	; before jumping to public routine
								; ebx contains handler address
								; this pointer points to function in C file kernel/traps.c
	jmp no_error_code			; enter public routine with eax as pointer


debug_1: ; vector 1, no error code
	push eax
	mov eax, do_debug_1
	jmp no_error_code


nmi_2: ; vector 2, no error code
	push eax
	mov eax, do_nmi_2
	jmp no_error_code

int3_3: ; vector 3, no error code
	push eax
	mov eax, do_int3_3
	jmp no_error_code


overflow_4: ; vector 4, no error code
	push eax
	mov eax, do_overflow_4
	jmp no_error_code


bounds_5: ; vector 5, no error code
	push eax
	mov eax, do_bounds_5
	jmp no_error_code


invalid_op_6: ; vector 6, no error code
	push eax
	mov eax, do_invalid_op_6
	jmp no_error_code


device_not_available_7: ; vector 7, no error code
	push eax
	mov eax, do_device_not_available_7
	jmp no_error_code


double_fault_8: ; vector 8, no error code
	push eax
	mov eax, do_double_fault_8
	jmp no_error_code


invalid_TSS_10: ; vector 10, error code
	xchg eax, [esp]		; eax contains error code
						; because SS points to same segment of DS
						; so do not need to specify SS:ESP
	push ebx
	mov ebx, do_invalid_TSS_10
	jmp error_code


segment_not_present_11: ; vector 11, error code
	xchg eax, [esp]	
	push ebx
	mov ebx, do_segment_not_present_11
	jmp error_code


stack_segment_error_12: ; vector 12, error code
	xchg eax, [esp]	
	push ebx
	mov ebx, do_stack_segment_error_12
	jmp error_code


general_protection_13: ; vector 13, error code
	xchg eax, [esp]	
	push ebx
	mov ebx, do_general_protection_13
	jmp error_code


page_fault_14: ; vector 14, error code
	xchg eax, [esp]	
	push ebx
	mov ebx, do_page_fault_14
	jmp error_code


coprocessor_error_16: ; vector 15, error code
	xchg eax, [esp]	
	push ebx
	mov ebx, do_coprocessor_error_16
	jmp error_code






; after entrance of routine
; +---------------+
; | 1.eax         | 
; +---------------+
; and eax contains handler address

; then this routine will proceed pushing
; to the layout as below
; +---------------+
; | 2.ebx         |
; +---------------+
; | 3.ecx         |
; +---------------+
; | 4.edx         |
; +---------------+
; | 5.edi         |
; +---------------+
; | 6.esi         |
; +---------------+
; | 7.ebp         |
; +---------------+
; | 8.ds          |
; +---------------+
; | 9.es          |
; +---------------+
; | 10.fs         |
; +---------------+
; | 11.error code | <- 0 as place holder
; +---------------+
; | 12.esp addr   | <- address of esp saved by hardware
; +---------------+

no_error_code: 
	push ebx
	push ecx
	push edx
	push edi
	push esi
	push ebp
	push ds
	push es
	push fs
	push 0				; push 0 as place holder of error code
						; maintain consistency of C function parameter position
	lea ebx, [esp+4*11]	; 11 resigers on the stack each size is 4 bytes
	push ebx			; pointer to "the" eip which was pushed by hardware after interrupt/exception

	mov dx, 0x10
	mov ds, dx
	mov es, dx

	call eax			; call handler(C function)

	add esp, 8			; discard last two push 
						; because no register should carry these value
	pop fs				; then pop other values on the stack and restore in reverse order
	pop es
	pop ds
	pop ebp
	pop esi
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax				; pop done

	iret






; after entrance of routine
; +---------------+
; | 1.eax         |
; +---------------+
; | 2.ebx         |
; +---------------+
; and eax contains error code, ebx contains handler address

; then this routine will proceed pushing
; to the layout as below
; +---------------+
; | 3.ecx         |
; +---------------+
; | 4.edx         |
; +---------------+
; | 5.edi         |
; +---------------+
; | 6.esi         |
; +---------------+
; | 7.ebp         |
; +---------------+
; | 8.ds          |
; +---------------+
; | 9.es          |
; +---------------+
; | 10.fs         |
; +---------------+
; | 11.error code | <- contained by eax
; +---------------+
; | 12.esp addr   | <- address of esp saved by hardware
; +---------------+

error_code: 
	push ecx
	push edx
	push edi
	push esi
	push ebp
	push ds
	push es
	push fs
	push eax			; eax contains error code
	lea eax, [esp+4*11]
	push eax			; pointer to "the" eip which was pushed by hardware after interrupt/exception

	mov ax, 0x10
	mov ds, ax
	mov es, ax

	call ebx			; call handler(C function) 

	add esp, 8			; discard last two push
	pop fs
	pop es
	pop ds
	pop esi
	pop ebp
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax

	iret

