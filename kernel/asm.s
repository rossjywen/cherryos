
extern divide_zero

section .text

; interrupt/exception with CPL change
; note: error code is optional due to manual

; +--------------+
; | 1.old SS     |
; +--------------+
; | 2.old ESP    |
; +--------------+
; | 3.old EFLAGS |
; +--------------+
; | 4.old CS     |
; +--------------+
; | 5.old EIP    | <- esp (without error code)
; +--------------+
; | 6.error code | <- esp (with error code)
; +--------------+









; vector: 0
; code: no error code
divide_zero: 
	push eax				; save eax
	mov eax, divide_zero	; before jumping to public routine
							; ebx contains handler address
							; this pointer points to function in C file kernel/traps.c
	jmp no_error_code		; enter public routine with eax as pointer





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

	call [eax]			; call C function

	sub esp, 8			; discard last two push on the stack 
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



;error_code:
;	xchg eax, [esp]
;	push ebx
;	;mov ebx, xxx





