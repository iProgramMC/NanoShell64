// NanoShell64 - Hardware abstraction.
// In the future this module is planned to be linked separately to the kernel.
#ifndef NS64_HAL_H
#define NS64_HAL_H

// ==== Terminal ====
void HalTerminalInit();
void HalDebugTerminalInit();
void HalPrintString(const char* str);
void HalPrintStringDebug(const char* str);

#endif//NS64_HAL_H
