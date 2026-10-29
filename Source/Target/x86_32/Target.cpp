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
/// \file Target.cpp
///
/// \brief Machine dependent code.
///
/// This file provides the low-level x86_32 specific hardware initializations,
/// POSIX system clock interfaces, and context stack layout tools for the Tics
/// RTOS.It allows Tics to run on a PC which greatly enhances productivity.
/// The typical development procedure to first develop on the PC in x86_32
/// mode, then switch to developing using QEMU to simulate your hardware, and
/// finally moving to the actual hardware.All 3 steps would use VS Code as
/// their development and debug system.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <stdlib.h>
#include <time.h>

//-----------------------------------------------------------------------------
/// \brief extern "C" function prototypes
//-----------------------------------------------------------------------------
extern "C"
{
    // A C function that trampolines to the Tics task when it first runs.
    void TrampolineToNewTask();

    // Raw low-level assembly query pulling the active hardware stack register.
    uintptr_t GetStackPointer();

    // Standard POSIX monotonic system clock function signature override.
    // Used to simulate a system tick for Tics timing mgmt while running
    // on the PC in x86_32 mode.
    int clock_gettime(clockid_t clockId, struct timespec *tp);
}

//-----------------------------------------------------------------------------
/// \brief Begin TicsNameSpace
//-----------------------------------------------------------------------------
namespace TicsNameSpace
{

//-----------------------------------------------------------------------------
/// Enums
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Structs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Globals
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Statics
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \brief Function definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \brief Perform hardware initializations
///
/// Intentionally empty for the x86_32 target configuration. Some targets
/// like the x86 PC need no special initializations, but since Tics calls
/// this function at initialization time, a dummy one must be provided
/// even when the processor requires no special initializations.

//-----------------------------------------------------------------------------
void TargetInit() {}

//-----------------------------------------------------------------------------
/// \brief This function returns the system tick count in milliseconds.
///
/// Tics requires a 1 ms tick count to manage timers. This function provides
/// a running tick count that wraps back to zero after the maximum value
/// is reached. Since we are running Linux, we just grab the time from the Linux
/// time function. In a real x86_32 embedded system this function would have
/// to be replaced.
///
/// \return The current 1 ms timer tick count value.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount()
{
    // Define the local storage structure for POSIX clock reading data.
    struct timespec tp;

    // The system tick count in ms.
    TimerTickType numMs = 0;

    // Sample the monotonic clock data directly from the Linux operating system.
    clock_gettime(CLOCK_MONOTONIC, &tp);

    // Compute the number of ms from the number of seconds.
    numMs += (tp.tv_sec * 1000);

    // Add the number of ns converted to ms obtain the total current tick count in ms.
    numMs += (tp.tv_nsec / 1000000);

    // Return the running system tick in milliseconds.
    return numMs;
}

//-----------------------------------------------------------------------------
/// \brief Return the current value of the stack pointer register.
///
/// This function returns the value of the stack pointer register before the
/// call was made. Any modifications performed on the sp should not be
/// included in the returned value.
///
/// \return The value of sp prior to the call.
//-----------------------------------------------------------------------------
StackType *GetTaskStackPointer()
{
    // Get the raw, unadjusted hardware stack pointer value.
    uintptr_t rawSp = GetStackPointer();

    // Compensate by adding the size of the return address that was pushed.
    uintptr_t adjustedSp = rawSp + sizeof(StackType);

    // Return the value of sp prior to the call.
    return (StackType *)adjustedSp;
}

//-----------------------------------------------------------------------------
/// \brief Class member functions.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \brief Primes a newly created task's stack with an initial values.
///
/// When a task first begins, it must restore registers that were previously
/// saved on the stack. However, the very first time the task runs, there are
/// no previously saved registers, so, the stack must be populated with values
/// that can be popped into the proper registers when the task is first run.
/// The purpose of this function is to populate the stack. The values
/// used to populate the stack can be any value, since they won't be used.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack()
{
    // Set sp one word above the first stack word. (Yes, StackTop points to the
    // word above the first stack word.)
    StackType *sp = StackTop;

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
