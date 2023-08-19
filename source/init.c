#include <_limine.h>
#include <main.h>
#include <hal.h>

// bootloader requests
volatile struct limine_hhdm_request g_HHDMRequest =
{
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = NULL,
};
volatile struct limine_framebuffer_request g_FramebufferRequest =
{
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = NULL,
};

// The entry point to our kernel.
void _start(void)
{
	HalDebugTerminalInit();
	SLogMsg("NanoShell64 is starting up");
	
	HalTerminalInit();
	LogMsg("NanoShell64 (TM), August 2023 - ReWrite - V0.001");
	
	while (true)
	{
		__asm__("hlt":::"memory");
	}
}
