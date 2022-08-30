#include <stdint.h>
#include <stddef.h>
#include <limine.h>

void WritePort (char port, char thing)
{
	__asm__ volatile ("out %%al, %%dx"::"a"(thing),"d"(port));
}

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static void done(void) {
    for (;;) {
        __asm__("hlt");
    }
}

// The following will be our kernel's entry point.
void _start(void) {
	
	WritePort(0xe9, 'N');
	
    // Ensure we got a terminal
    if (terminal_request.response == NULL
     || terminal_request.response->terminal_count < 1) {
        done();
    }

	
	WritePort(0xe9, 'N');
    // We should now be able to call the Limine terminal to print out
    // a simple "Hello World" to screen.
    struct limine_terminal *terminal = terminal_request.response->terminals[0];
    terminal_request.response->write(terminal, "Hello World", 11);

	
	WritePort(0xe9, 'N');
    // We're done, just hang...
    done();
}
