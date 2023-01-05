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
	//LockGuard lg(g_E9Spinlock);
	while (*str)
	{
		Arch::WriteByte(0xE9, *str);
		str++;
	}
}

void Terminal::Write(const char* str)
{
	LockGuard lg(g_TermSpinlock);
	g_TerminalRequest.response->write(g_TerminalRequest.response->terminals[0], str, strlen(str));
}
