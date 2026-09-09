/*
MIT License

Copyright (c) 2026 Michael Dennis McDonnell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//-----------------------------------------------------------------------------
// ARM Cortex-M (Thumb-2) Specific C++ Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
// Cortex-M SysTick Memory-Mapped Registers (Standard ARM Core Peripherals)
#define ARM_SYSTICK_CTRL (*(volatile uint32_t *)0xE000E010)
#define ARM_SYSTICK_LOAD (*(volatile uint32_t *)0xE000E014)
#define ARM_SYSTICK_VAL (*(volatile uint32_t *)0xE000E018)

//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------
extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();

#include <stdio.h>
#include <sys/types.h>

extern "C"
{
    void _exit(int status)
    {
        while (1)
            ;
    }
    int _close(int file) { return -1; }
    int _fstat(int file, void *st) { return -1; }
    int _isatty(int file) { return 1; }
    int _lseek(int file, int ptr, int dir) { return 0; }
    int _read(int file, char *ptr, int len) { return 0; }
    int _write(int file, char *ptr, int len) { return len; }
    caddr_t _sbrk(int incr)
    {
        extern char _ebss;
        static char *heap_end = &_ebss;
        char *prev_heap_end = heap_end;
        heap_end += incr;
        return (caddr_t)prev_heap_end;
    }
}

// Global monotonic millisecond counter incremented by SysTick ISR
static volatile uint32_t FreeRunningMsCounter = 0;

// C-linkage bridge called by SysTick_Handler in Target.s
extern "C" void TimerTickIsr() { FreeRunningMsCounter++; }

//-----------------------------------------------------------------------------
// Namespace
//-----------------------------------------------------------------------------
namespace TicsNameSpace
{

//-----------------------------------------------------------------------------
/// \brief Returns the millisecond system tick count.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount() { return (TimerTickType)FreeRunningMsCounter; }

//-----------------------------------------------------------------------------
/// \brief Primes a newly created task's stack for ARM Cortex-M.
///
/// When a task is first run, the registers saved during TaskSwitch must be
/// restored via ldmia.w sp!, {r4-r11, pc}. This primes the stack so the very
/// first context switch lands correctly in TrampolineToNewTask.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack()
{
    // 1. Start directly at the raw top of the allocated stack memory pool
    // Ensure 8-byte stack alignment required by ARM AAPCS
    uint32_t *sp = (uint32_t *)((uintptr_t)StackTop & ~7);

    // 2. PUSH CONTEXT (Mapping to ARM ldmia.w sp!, {r4-r11, pc})
    // ldmia loads lowest address first (r4) up to highest address (pc).
    // Therefore, the item pushed first chronologically (highest address) is pc.
    *(--sp) = (uint32_t)(uintptr_t)TrampolineToNewTask; // PC (saved LR)

    // Callee-saved registers r11 down to r4
    *(--sp) = 11; // r11 (fp)
    *(--sp) = 10; // r10 (sl)
    *(--sp) = 9;  // r9
    *(--sp) = 8;  // r8
    *(--sp) = 7;  // r7
    *(--sp) = 6;  // r6
    *(--sp) = 5;  // r5
    *(--sp) = 4;  // r4

    // 3. Save the finalized stack pointer position to this task's Stack.SavedSp variable.
    SavedSp = (StackType *)sp;
    return;
}

} // namespace TicsNameSpace
