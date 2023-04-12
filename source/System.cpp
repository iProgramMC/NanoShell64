//  ***************************************************************
//  System.cpp - Creation date: 05/01/2023
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************
#include <Nanoshell.hpp>

extern "C" void AssertUnreachable(const char* src_file, int src_line)
{
	KernelPanic("ASSERT_UNREACHABLE reached at %s:%d", src_file, src_line);
}
