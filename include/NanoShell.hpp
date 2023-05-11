//  ***************************************************************
//  Nanoshell.hpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#ifndef _NANOSHELL_HPP
#define _NANOSHELL_HPP

// This file includes global definitions that everyone should have.
#include <cstdint>
#include <cstddef>
#include <cstdarg>

#define PACKED        __attribute__((packed))
#define NO_RETURN     __attribute__((noreturn))
#define RETURNS_TWICE __attribute__((returns_twice))
#define UNUSED        __attribute__((unused))

inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

class nopanic_t {};

constexpr nopanic_t nopanic;

void* operator new(size_t, const nopanic_t&);
void* operator new[](size_t, const nopanic_t&);

#define ASM __asm__ __volatile__

#ifdef TARGET_X86_64
struct Registers
{
	uint16_t ds, es, fs, gs;
	uint64_t cr2;
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rbp, rdi, rsi;
	uint64_t rdx, rcx, rbx, rax;
	uint64_t error_code;
	uint64_t rip, cs, rflags, rsp, ss; // pushed by the ISR and popped by iretq.
}
PACKED;
#endif

extern "C"
{
	void* memcpy(void* dst, const void* src, size_t n);
	void* memquadcpy(uint64_t* dst, const uint64_t* src, size_t n);
	void* memset(void* dst, int c, size_t n);
	char* strcpy(char* dst, const char* src);
	char* strcat(char* dst, const char* src);
	int strcmp(const char* s1, const char* s2);
	int memcmp(const void* s1, const void* s2, size_t n);
	size_t strlen(const char * s);
	int vsnprintf(char* buf, size_t sz, const char* fmt, va_list arg);
	int snprintf(char* buf, size_t sz, const char* fmt, ...);
	int sprintf(char* buf, const char* fmt, ...);

	void LogMsg(const char* fmt, ...);
	void LogMsgNoCR(const char* fmt, ...);

	void SLogMsg(const char* fmt, ...);
	void SLogMsgNoCR(const char* fmt, ...);
	
	NO_RETURN void KernelPanic(const char* fmt, ...);
	
	NO_RETURN void AssertUnreachable(const char* src_file, int src_line);
	
	#define ASSERT_UNREACHABLE AssertUnreachable(__FILE__, __LINE__)
};

#endif//_NANOSHELL_HPP
