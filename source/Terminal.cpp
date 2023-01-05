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
#include <Arch.hpp>
#include <_limine.h>

// TODO: Restructure this so that it's based on a worker thread. For now, this works.

volatile limine_terminal_request g_TerminalRequest =
{
	.id = LIMINE_TERMINAL_REQUEST,
	.revision = 0,
	.response = NULL,
	.callback = NULL,
};

bool Terminal::CheckResponse()
{
	return g_TerminalRequest.response != NULL;
}

Spinlock g_E9Spinlock;
Spinlock g_TermSpinlock;

void Terminal::E9Write(const char* str)
{
	LockGuard lg(g_E9Spinlock);
	while (*str)
	{
		Arch::WriteByte(0xE9, *str);
		str++;
	}
}

void Terminal::E9WriteLn(const char* str)
{
	LockGuard lg(g_E9Spinlock);
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
	g_TerminalRequest.response->write(g_TerminalRequest.response->terminals[0], str, strlen(str));
}

void Terminal::WriteLn(const char* str)
{
	LockGuard lg(g_TermSpinlock);
	auto resp = g_TerminalRequest.response;
	auto term = g_TerminalRequest.response->terminals[0];
	resp->write(term, str, strlen(str));
	resp->write(term, "\n", 1);
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
