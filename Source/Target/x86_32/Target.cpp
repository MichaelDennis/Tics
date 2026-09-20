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
// This file provides the low-level x86_32 specific hardware initializations,
// POSIX system clock interfaces, and context stack layout tools for the Tics
// RTOS.It allows Tics to run on a PC which greatly enhances productivity.
// The typical development procedure to first develop on the PC in x86_32
// mode, then switch to developing using QEMU to simulate your hardware, and
// finally moving to the actual hardware.All 3 steps would use VS Code as
// their development and debug system.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <stdlib.h>
#include <time.h>

//-----------------------------------------------------------------------------
// extern "C"
//-----------------------------------------------------------------------------
extern "C"
{
    // A C function that trampolines to the Tics task when it first runs.
    void TrampolineToNewTask();

    // Raw low-level assembly query pulling the active hardware stack register.
    uintptr_t GetStackPointer();

    // Standard POSIX monotonic system clock function signature override.
    // Used to simulate a system tick for Tics timing mgmt.
    int clock_gettime(clockid_t clockId, struct timespec *tp);
}

//-----------------------------------------------------------------------------
// Start TicsNameSpace
//-----------------------------------------------------------------------------
namespace TicsNameSpace
{

//-----------------------------------------------------------------------------
// Enums
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Statics
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Centralized hardware initialization entry target execution point.
//-----------------------------------------------------------------------------
void TargetInit()
{
    // Intentionally empty for the x86_32 target configuration. Some targets
    // like the x86 PC need no special initializations, but since Tics calls
    // this function at initialization time, a dummy one must be provided
    // even when the processor requires no special initializations.
}

//-----------------------------------------------------------------------------
// This function returns the system tick count in milliseconds.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount()
{
    // Define the local storage structure for POSIX clock reading data.
    struct timespec tp;

    // Sample the monotonic clock data directly from the Linux operating system.
    clock_gettime(CLOCK_MONOTONIC, &tp);

    // Compute the cumulative tick count in milliseconds.
    return (TimerTickType)(tp.tv_sec * 1000 + (tp.tv_nsec / 1000000));
}

//-----------------------------------------------------------------------------
// This target-specific wrapper handles the calling-convention compensation
// math and returns the clean stack pointer (SP) value of the current task.
//-----------------------------------------------------------------------------
StackType *GetTaskStackPointer()
{
    // Query the raw, unadjusted hardware stack pointer value from assembly.
    uintptr_t rawSp = GetStackPointer();

    // Compensate by adding the size of the return address that was pushed.
    uintptr_t adjustedSp = rawSp + sizeof(StackType);

    // Return the clean, scaled address pointer back to the generic scheduler.
    return (StackType *)adjustedSp;
}

//-----------------------------------------------------------------------------
// Classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Primes a newly created task's stack with an initial dummy register context.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack()
{
    // This variable will hold the address of the top of the stack.
    StackType rawSp;

    // Assign the pointer to the top of the stack.
    rawSp = (StackType)StackTop;

    // Mask out unaligned address layers to guarantee a strict 16-byte boundary alignment layout.
    rawSp &= SixteenByteBoundaryMask;

    // Save the corrected aligned address directly back into the stack top variable.
    StackTop = (StackType *)rawSp;

    // Initialize the working stack pointer that will be used below.
    StackType *sp = (StackType *)rawSp;

    // Populate the stack frame with initial placeholder register values.
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = (StackType)TrampolineToNewTask;
    *(--sp) = 0;
    *(--sp) = 1;
    *(--sp) = 2;
    *(--sp) = 3;

    // Save the sp for use in a subsequent context switch.
    SavedSp = sp;

    // Conclude execution and exit the stack layout setup routine.
    return;
}

} // namespace TicsNameSpace
