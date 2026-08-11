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
// Riscv-32 Specific C++ Functions
//
// This file includes all Riscv-32 specific functions that can be written in 
// C++. Functions that must be written assembly language are in file 
// Target.s.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp" 
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

// The Riscv32 timer "register".
#define RV32_CLINT_MTIME_ADDR (*(volatile uint32_t*)0x0200BFF8)

//-----------------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------------

// A C function that trampolines to the Tics error handler.
extern "C" void TrampolineToErrorHandler();

// A C function that trampolines to the task function when a task is started.
extern "C" void TrampolineToNewTask();

//-----------------------------------------------------------------------------
// Namespace
//-----------------------------------------------------------------------------
namespace TicsNameSpace {

//-----------------------------------------------------------------------------
/// \brief Returns the millisecond system tick count.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount() 
{
    // Persistent storage for the last sampled clock edge, measured in raw ticks (1 tick = 100 nanoseconds / 0.1 microseconds).
    static uint32_t lastRawHardwareTicks = 0;

    // Accumulation bucket for fractional tick remainders that have passed since the last whole millisecond boundary.
    static uint32_t subMsTicksBucket = 0;

    // Global monotonic system timeline counting upward, measured in whole milliseconds (1 ms = 1,000,000 nanoseconds).
    static uint32_t freeRunningMsCounter = 0;

    // Dereference the physical, memory-mapped CLINT timer register tracking the continuous 10MHz hardware oscillator.
    uint32_t currentRawHardwareTicks = RV32_CLINT_MTIME_ADDR;

    // Calculate the raw ticks elapsed since the last function execution, natively handling any 32-bit integer overflows.
    uint32_t elapsedTicks = currentRawHardwareTicks - lastRawHardwareTicks;

    // Cache the most recent hardware clock snapshot into permanent memory to establish the baseline for the next poll pass.
    lastRawHardwareTicks = currentRawHardwareTicks;

    // Deposit the freshly harvested slice of raw execution ticks directly into the sub-millisecond remainder storage bucket.
    subMsTicksBucket += elapsedTicks;

    // Check if the accumulated remainder bucket contains enough raw energy to cross at least a single 1 millisecond threshold.
    if (subMsTicksBucket >= 10000) {
        // Execute direct integer division to compute exactly how many whole milliseconds have elapsed (10,000 raw ticks = 1 ms).
        uint32_t msPassed = subMsTicksBucket / 10000;

        // Advance the master free-running clock timeline by the exact number of verified whole milliseconds that just passed.
        freeRunningMsCounter += msPassed;

        // Apply a modulo operation to cleanly drain the consumed milliseconds and preserve the remaining fractional ticks.
        subMsTicksBucket %= 10000;
    }

    // Cast the permanent 32-bit millisecond tracking integer to your customized type definition and return it to the scheduler.
    return (TimerTickType)freeRunningMsCounter;
}

//-----------------------------------------------------------------------------
/// \brief Primes a newly created task's stack.
///
/// When a task is first run, it's registers that were saved just before the 
/// task was swtiched out to let another task run, must be restored. This is done
/// by copying the saved registers off the stack and writing the values back
/// to the registers. Well, the very first time the task is run, there is
/// no previous time when registers were saved to the stack. So, we have
/// to write register values to the stack so that the very first time
/// the task is run, there are registers on the stack so that the task
/// switching code can run properly. The register values written to the stack
/// are completely arbitrary.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack() 
{
    // 1. Start directly at the raw top of the allocated stack memory pool
    uint32_t *sp = (uint32_t *) StackTop;

    // 2. PUSH CONTEXT (Pure sequential pointer decrements mapping directly to Target.s)
    *(--sp) = 11; // s11 (Chronologically first push, highest address)
    *(--sp) = 10; // s10
    *(--sp) = 9;  // s9
    *(--sp) = 8;  // s8
    *(--sp) = 7;  // s7
    *(--sp) = 6;  // s6
    *(--sp) = 5;  // s5
    *(--sp) = 4;  // s4
    *(--sp) = 3;  // s3
    *(--sp) = 2;  // s2
    *(--sp) = 1;  // s1
    *(--sp) = 0;  // s0
    
    // The return address, register ra, is at the lowest memory address (Chronologically last push)
    *(--sp) = (uint32_t)(uintptr_t)TrampolineToNewTask; 

    // 3. Save the finalized stack pointer position to this task's Stack.SavedSp variable.
    SavedSp = (StackType *) sp;
    return;
}

} // namespace TicsNameSpace
