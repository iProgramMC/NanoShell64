//  ***************************************************************
//  Terminal.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#include <Terminal.hpp>
#include <Spinlock.hpp>
#include <EternalHeap.hpp>
#include <Arch.hpp>
#include <_limine.h>
#include "LimineTerm/framebuffer.h"

// TODO: Restructure this so that it's based on a worker thread. For now, this works.

volatile limine_framebuffer_request g_FramebufferRequest =
{
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = NULL,
};

term_context* g_pTermContext = NULL;

bool Terminal::CheckResponse()
{
	return g_FramebufferRequest.response != NULL;
}

void Terminal::Setup()
{
	if (!CheckResponse()) Arch::IdleLoop();
	if (g_FramebufferRequest.response->framebuffer_count == 0) Arch::IdleLoop();
	
	limine_framebuffer* pFB = g_FramebufferRequest.response->framebuffers[0];
	
	uint32_t defaultBG = 0x0000007f;
	uint32_t defaultFG = 0x00ffffff;
	
	g_pTermContext = fbterm_init(
		EternalHeap::Allocate,
		(uint32_t*)pFB->address,
		pFB->width, pFB->height, pFB->pitch,
		NULL,       // canvas
		NULL,       // ANSI colors
		NULL,       // ANSI bright colors
		&defaultBG, // Default BG
		&defaultFG, // Default FG
		NULL,       // Default BG Bright
		NULL,       // Default FG Bright
		NULL,       // Font
		0, 0,       // font width, height
		0,          // font spacing
		1, 1,       // font scale X, Y
		0
	);
}

Spinlock g_E9Spinlock;
Spinlock g_TermSpinlock;

void Terminal::E9Write(const char* str)
{
	//LockGuard lg(g_E9Spinlock);
	while (*str)
	{
		Arch::WriteByte(0xE9, *str);
		str++;
	}
}

void Terminal::E9WriteLn(const char* str)
{
	//LockGuard lg(g_E9Spinlock);
	while (*str)
	{
		Arch::WriteByte(0xE9, *str);
		str++;
	}
	Arch::WriteByte(0xE9, '\n');
}

void Terminal::Write(const char* str)
{
	LockGuard lg(g_TermSpinlock);
	term_write(g_pTermContext, str, strlen(str));
}

void Terminal::WriteLn(const char* str)
{
	LockGuard lg(g_TermSpinlock);
	term_write(g_pTermContext, str, strlen(str));
	term_write(g_pTermContext, "\n", 1);
}

// LogMsg and SLogMsg. Useful for debugging, these can be called from anywhere, even C contexts.
// Note: This is limited to 1024 bytes. Careful!
// TODO: Don't repeat myself
// TODO: Expand this maybe?
extern "C" void LogMsg(const char* fmt, ...)
{
	va_list lst;
	va_start(lst, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof buffer, fmt, lst);
	va_end(lst);
	
	Terminal::WriteLn(buffer);
}

extern "C" void SLogMsg(const char* fmt, ...)
{
	va_list lst;
	va_start(lst, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof buffer, fmt, lst);
	va_end(lst);
	
	Terminal::E9WriteLn(buffer);
}

extern "C" void LogMsgNoCR(const char* fmt, ...)
{
	va_list lst;
	va_start(lst, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof buffer, fmt, lst);
	va_end(lst);
	
	Terminal::Write(buffer);
}

extern "C" void SLogMsgNoCR(const char* fmt, ...)
{
	va_list lst;
	va_start(lst, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof buffer, fmt, lst);
	va_end(lst);
	
	Terminal::E9Write(buffer);
}
