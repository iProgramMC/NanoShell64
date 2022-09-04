//  ***************************************************************
//  VirtMemMgr.cpp - Creation date: 03/09/2022
//  -------------------------------------------------------------
//  NanoShell64 Copyright (C) 2022 - Licensed under GPL V3
//
//  ***************************************************************
//  Programmer(s):  iProgramInCpp (iprogramincpp@gmail.com)
//  ***************************************************************

#include <stdint.h>
#include <stddef.h>
#include <_limine.h>

volatile limine_hhdm_request g_hhdm_request =
{
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
	.response = NULL,
};

volatile limine_kernel_address_request g_kaddr_request =
{
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
	.response = NULL,
};
