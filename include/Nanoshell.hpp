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
#define NO_RETURN     __attribute__((no_return))
#define RETURNS_TWICE __attribute__((returns_twice))
#define UNUSED        __attribute__((unused))

inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

#endif//_NANOSHELL_HPP
