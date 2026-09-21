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
// This file provides the low-level RISC-V32 specific hardware initializations,
// CLINT timer tracking drivers, and context stack layout tools for the Tics RTOS.
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

    // Raw low-level assembly query pulling the active hardware stack register.
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

// Dereference pointer for the physical, memory-mapped CLINT timer register tracking the continuous
// 10MHz hardware oscillator.
static volatile uint32_t *const Rv32ClintMtimeAddr = (volatile uint32_t *)0x0200BFF8;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Centralized hardware initialization entry target execution point.
//-----------------------------------------------------------------------------
void TargetInit()
{
    // Intentionally empty for the RISC-V32 target configuration.
}

//-----------------------------------------------------------------------------
// Read-interface query layer pulling and accumulating the millisecond system tick count.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount()
{
    // Persistent storage for the last sampled clock edge, measured in raw ticks (1 tick = 100
    // nanoseconds / 0.1 microseconds).
    static uint32_t lastRawHardwareTicks = 0;

    // Accumulation bucket for fractional tick remainders that have passed since the last whole
    // millisecond boundary.
    static uint32_t subMsTicksBucket = 0;

    // Global monotonic system timeline counting upward, measured in whole milliseconds (1 ms =
    // 1,000,000 nanoseconds).
    static uint32_t freeRunningMsCounter = 0;

    // Sample the hardware timer tracking address.
    uint32_t currentRawHardwareTicks = *Rv32ClintMtimeAddr;

    // Calculate the raw ticks elapsed since the last function execution, natively handling any
    // 32-bit integer overflows.
    uint32_t elapsedTicks = currentRawHardwareTicks - lastRawHardwareTicks;

    // Cache the most recent hardware clock snapshot into permanent memory to establish the baseline
    // for the next poll pass.
    lastRawHardwareTicks = currentRawHardwareTicks;

    // Deposit the freshly harvested slice of raw execution ticks directly into the sub-millisecond
    // remainder storage bucket.
    subMsTicksBucket += elapsedTicks;

    // Check if the accumulated remainder bucket contains enough raw energy to cross at least a
    // single 1 millisecond threshold.
    if (subMsTicksBucket >= 10000)
    {
        // Execute direct integer division to compute exactly how many whole milliseconds have
        // elapsed (10,000 raw ticks = 1 ms).
        uint32_t msPassed = subMsTicksBucket / 10000;

        // Advance the master free-running clock timeline by the exact number of verified whole
        // milliseconds that just passed.
        freeRunningMsCounter += msPassed;

        // Apply a modulo operation to cleanly drain the consumed milliseconds and preserve the
        // remaining fractional ticks.
        subMsTicksBucket %= 10000;
    }

    // Cast the permanent 32-bit millisecond tracking integer to your customized type definition and
    // return it to the scheduler.
    return (TimerTickType)freeRunningMsCounter;
}

//-----------------------------------------------------------------------------
// This target-specific wrapper handles the calling-convention compensation
// math and returns the clean stack pointer (SP) value of the current task.
//-----------------------------------------------------------------------------
StackType *GetTaskStackPointer()
{
    // Query the raw, unadjusted hardware stack pointer value from assembly.
    uintptr_t rawSp = GetStackPointer();

    // Return the raw address directly as no stack compensation is needed on RISC-V32.
    return (StackType *)rawSp;
}

//-----------------------------------------------------------------------------
// Classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Primes a newly created task's stack with an initial dummy register context.
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
    *(--sp) = (StackType)TrampolineToNewTask; // ra return address pointer.

    // Save the finalized stack pointer position to this task's Stack.SavedSp variable.
    SavedSp = (StackType *)sp;
}

} // namespace TicsNameSpace
