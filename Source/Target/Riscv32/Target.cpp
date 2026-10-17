/*
MIT License

Copyright (c) 2026 Michael Dennis McDonnell (Tics Realtime)

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
// This file provides code that generates a 1 ms system timer tick used
// by the Tics RTOS.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <stdlib.h>

//-----------------------------------------------------------------------------
// extern "C"
//-----------------------------------------------------------------------------
extern "C"
{
    // A C function that trampolines to the Tics error handler.
    void TrampolineToErrorHandler();

    // A C function that trampolines to the task function when a task is started.
    void TrampolineToNewTask();

    // Assembly language function that returns the value of the current stack pointer.
    // See it in file Target.s.
    uintptr_t GetStackPointer();
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

// Get a pointer to the raw nanosecond clock tick source. This will be used to
// compute a running millisecond clock tick in function GetSystemTickCount (see
// below).
static volatile uint32_t *const Rv32ClintMtimeAddr = (volatile uint32_t *)0x0200BFF8;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This function performs all necessary hardware initializations.
//-----------------------------------------------------------------------------
void TargetInit()
{
    // This is a general function to perform any necessary initializations.
    // No initializations are needed here. Not all hardware targets require
    // initializations.
}

//-----------------------------------------------------------------------------
// Update and return the ongoing millisecond system tick count.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount()
{
    // The hardware tick reading saved from the previous call.
    static uint32_t lastRawHardwareTicks = 0;

    // Any ticks past the ms mark that were saved from the last call.
    static uint32_t subMsTicksBucket = 0;

    // The ongoing count in milliseconds (ms) that is returned to the caller.
    // This value is saved across calls.
    static uint32_t freeRunningMsCounter = 0;

    // Read the raw hardware tick count in nanoseconds (ns).
    uint32_t currentRawHardwareTicks = *Rv32ClintMtimeAddr;

    // Compute the elapsed tick count since the last reading.
    uint32_t elapsedTicks = currentRawHardwareTicks - lastRawHardwareTicks;

    // Update the last number of raw ticks read.
    lastRawHardwareTicks = currentRawHardwareTicks;

    // Since we read more that 100 ms worth of ticks last time, we saved them
    // in subMsTicksBucket. Now we update it with the number of elapsed ticks
    // we just read.
    subMsTicksBucket += elapsedTicks;

    // if the subMsTicksBucket is >= 10,000, then the elapsed time is 1 or more ms.
    if (subMsTicksBucket >= 10000)
    {
        // Compute the elapsed number of ms by dividing by 10,000 ns per ms.
        uint32_t msPassed = subMsTicksBucket / 10000;

        // Update the ongoing number of elapsed ms.
        freeRunningMsCounter += msPassed;

        // Save the left over ns count in subMsTicksBucket.
        subMsTicksBucket %= 10000;
    }

    // Return the ongoing ms count, which may be the same as it was in the last call
    // depending upon the number of ns that have elapsed since the last call.
    return (TimerTickType)freeRunningMsCounter;
}

//-----------------------------------------------------------------------------
// This target-specific wrapper handles the calling-convention compensation
// math and returns the clean stack pointer (SP) value of the current task.
//-----------------------------------------------------------------------------
StackType *GetTaskStackPointer()
{
    // Read the raw, unadjusted hardware stack pointer value.
    uintptr_t rawSp = GetStackPointer();

    // Return the raw address directly as no stack compensation is needed on RISC-V32.
    return (StackType *)rawSp;
}

//-----------------------------------------------------------------------------
// Classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This function primes a newly created task's stack with dummy values.
// It doesn't matter what the values are, since the values will be popped into
// registers that will be overwritten by the task that we context switch into.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack()
{
    // Start directly at the raw top of the allocated stack memory pool.
    StackType *sp = (StackType *)StackTop;

    // 2. Populate the stack frame with initial placeholder register values.
    *(--sp) = 11;                             // s11 register placeholder.
    *(--sp) = 10;                             // s10 register placeholder.
    *(--sp) = 9;                              // s9 register placeholder.
    *(--sp) = 8;                              // s8 register placeholder.
    *(--sp) = 7;                              // s7 register placeholder.
    *(--sp) = 6;                              // s6 register placeholder.
    *(--sp) = 5;                              // s5 register placeholder.
    *(--sp) = 4;                              // s4 register placeholder.
    *(--sp) = 3;                              // s3 register placeholder.
    *(--sp) = 2;                              // s2 register placeholder.
    *(--sp) = 1;                              // s1 register placeholder.
    *(--sp) = 0;                              // s0 register placeholder.
    *(--sp) = (StackType)TrampolineToNewTask; // The address of the trampoline function.

    // Save the finalized stack pointer position to this task's Stack.SavedSp variable.
    // This value will be loaded into the sp register during a context switch.
    SavedSp = (StackType *)sp;
}

} // namespace TicsNameSpace
