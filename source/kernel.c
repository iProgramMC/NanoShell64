#include <stdint.h>
#include <stddef.h>
#include <limine.h>

void WritePort (unsigned short port, unsigned char thing)
{
	__asm__ volatile ("out %%al, %%dx"::"a"(thing),"d"(port));
}

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_terminal_request g_TerminalRequest =
{
	.id = LIMINE_TERMINAL_REQUEST,
	.revision = 0
};
/*
static volatile struct limine_bootloader_info_request g_BootloaderInfoRequest =
{
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
	.revision = 0
};
*/
void KeStopSystem(void)
{
	__asm__("cli");
	
	for (;;) {
		__asm__("hlt");
	}
}

// The following will be our kernel's entry point.
void _start(void)
{
	WritePort(0xe9, 'N');
	
	// Ensure we got a terminal
	if (g_TerminalRequest.response == NULL)
	{
		WritePort(0xe9, 'k');
		KeStopSystem();
	}
	if (g_TerminalRequest.response->terminal_count < 1)
	{
		WritePort(0xe9, 'd');
		KeStopSystem();
	}
	
	WritePort(0xe9, 'A');
	
	// We should now be able to call the Limine terminal to print out
	// a simple "Hello World" to screen.
	struct limine_terminal *pTerminal = g_TerminalRequest.response->terminals[0];
	g_TerminalRequest.response->write(pTerminal, "Hello World", 11);

	WritePort(0xe9, 'X');
	
	// We're done, just hang...
	KeStopSystem();
}
