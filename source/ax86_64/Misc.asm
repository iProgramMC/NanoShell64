; ***************************************************************
; Misc.asm - Creation date: 06/01/2023
; -------------------------------------------------------------
; NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
;
; ***************************************************************
;  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
; ***************************************************************
;  
;  Module description:
;     This module implements interrupt routines and other stuff
;  that's hard to do within the C++ environment.
;
; ***************************************************************

bits 64

global Arch_APIC_OnInterrupt_Asm
extern Arch_APIC_OnInterrupt

%macro PUSH_ALL 0
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push rsp
%endmacro

%macro POP_ALL 0
	pop rsp
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rbp
	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

; Implements the assembly stub which calls into the C function, which then calls into the C++ function.
Arch_APIC_OnInterrupt_Asm:
	PUSH_ALL
	
	mov  rdi, rsp
	call Arch_APIC_OnInterrupt
	
	POP_ALL
	iretq
	

