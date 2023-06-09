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

global Arch_APIC_OnTimerInterrupt_Asm
extern Arch_APIC_OnTimerInterrupt
global Arch_APIC_OnIPInterrupt_Asm
extern Arch_APIC_OnIPInterrupt
global Arch_APIC_OnSpInterrupt_Asm
extern Arch_APIC_OnSpInterrupt

global CPU_OnPageFault_Asm
extern CPU_OnPageFault

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
	mov  rax, cr2
	push rax
	mov  ax, gs
	push ax
	mov  ax, fs
	push ax
	mov  ax, es
	push ax
%endmacro

%macro POP_ALL 0
	pop ax
	mov ax, es
	pop ax
	mov ax, fs
	pop ax
	mov ax, gs
	pop rax ; we don't need to uselessly pop cr2.
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
	add rsp, 8 ; we're supposed to add 8 to rsp anyways.
%endmacro

%macro PUSH_ALL_NO_ERC 0
	push 0           ; push a fake error code
	PUSH_ALL
%endmacro

; Swaps GS if needed, pushes DS.
%macro SWAP_GS_IF_NEEDED 0
	; swap gs if needed
	mov  ax, ds
	push ax
	cmp  ax, 0x40
	jne  .noneedtoswap
	swapgs
.noneedtoswap:
%endmacro

; Swaps GS back if needed, pops DS.
%macro SWAP_GS_BACK_IF_NEEDED 0
	pop  ax
	mov  ds, ax
	cmp  ax, 0x40
	jne  .noneedtoswap2
	swapgs
.noneedtoswap2:
%endmacro

CPU_OnPageFault_Asm:
	PUSH_ALL
	SWAP_GS_IF_NEEDED
	
	mov  rdi, rsp
	call CPU_OnPageFault
	
	SWAP_GS_BACK_IF_NEEDED
	POP_ALL
	iretq

; Implements the assembly stub which calls into the C function, which then calls into the C++ function.
Arch_APIC_OnIPInterrupt_Asm:
	PUSH_ALL_NO_ERC
	SWAP_GS_IF_NEEDED
	
	mov  rdi, rsp
	call Arch_APIC_OnIPInterrupt
	
	SWAP_GS_BACK_IF_NEEDED
	POP_ALL
	iretq

; Implements the assembly stub which calls into the C function, which then calls into the C++ function.
Arch_APIC_OnTimerInterrupt_Asm:
	PUSH_ALL_NO_ERC
	SWAP_GS_IF_NEEDED
	
	mov  rdi, rsp
	call Arch_APIC_OnTimerInterrupt
	
	SWAP_GS_BACK_IF_NEEDED
	POP_ALL
	iretq
	
; it's probably fine if we get a spurious interrupt. We don't really need to handle it
Arch_APIC_OnSpInterrupt_Asm:
	PUSH_ALL_NO_ERC
	SWAP_GS_IF_NEEDED
	
	mov  rdi, rsp
	call Arch_APIC_OnSpInterrupt
	
	SWAP_GS_BACK_IF_NEEDED
	POP_ALL
	iretq

; definition: NO_RETURN void JumpThreadEC( &ThreadExecutionContext );
global JumpThreadEC
JumpThreadEC:
	mov  rsp, rdi ; argument #1
	mov  rax, rsi ; argument #2. This will make SetThreadEC return this value a second time, like setjmp() would.
	or   rax, rax
	; failsafe to make sure we aren't just passing 0
	jnz  .not_zero
	inc  rax
.not_zero:
	pop  rbp
	pop  rbx
	pop  r12
	pop  r13
	pop  r14
	pop  r15
	iretq

; definition: RETURNS_TWICE uint64_t SetThreadEC( &ThreadExecutionContext );
global SetThreadEC
SetThreadEC:
	mov [rdi],      rbp ; preserve the registers that the 
	mov [rdi + 8],  rbx
	mov [rdi + 16], r12
	mov [rdi + 24], r13
	mov [rdi + 32], r14
	mov [rdi + 40], r15
	mov rax, [rsp]      ; get EIP after our call insn (return address)
	mov [rdi + 48], rax ; rip field of TEC
	mov rax, cs         ; get the code segment
	mov [rdi + 56], rax ; cs field of TEC
	pushfq              ; get the flags register
	mov rax, [rsp]      ; get it into rax
	add rsp, 8          ; undo pushfq's effects
	mov [rdi + 64], rax ; rflags field of TEC
	mov rax, rsp        ; get the rsp into eax
	add rax, 8          ; we need to do this, since we don't want to go to our
	                    ; return address and have the context of this function call...
	mov [rdi + 72], rax ; rsp field of the TEC
	mov rax, ss         ; get the stack segment
	mov [rdi + 80], rax ; ss field of TEC
	xor rax, rax        ; clear RAX for now. When a second return happens,
	                    ; JumpToThreadEC will set rax to something differe
	ret                 ; our job here is done.
	