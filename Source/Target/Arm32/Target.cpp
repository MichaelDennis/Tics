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
// This file provides the low-level ARM32 Cortex-M3 target specific hardware
// initializations, timer tracking drivers, and required system stub functions
// for the Tics RTOS engine.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <stdint.h>
#include <sys/types.h>

//-----------------------------------------------------------------------------
// extern "C"
//-----------------------------------------------------------------------------
extern "C"
{
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
// Core register constants and hardware clock metrics for the SysTick module.
//-----------------------------------------------------------------------------
enum SysTickPeripheralEnum : uint32_t
{
    // The base memory address for the System Control Space.
    ScsBaseAddr = 0xE000E000UL,

    // The base memory address for the SysTick peripheral registers.
    SysTickBaseAddr = (ScsBaseAddr + 0x0010UL),

    // Bit mask to enable the SysTick counter module.
    SysTickEnable = (1UL << 0),

    // Bit mask to enable the SysTick periodic exception interrupt.
    SysTickTickInt = (1UL << 1),

    // Bit mask to configure SysTick to use the core processor clock source.
    SysTickClkSource = (1UL << 2),

    // The running frequency of the CPU clock configuration under QEMU mps2-an385.
    CpuClockHz = 25000000UL,

    // The calculated count register value required to trigger a steady 1ms interval tick.
    SysTickReload1Ms = ((CpuClockHz / 1000UL) - 1UL),
};

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Hardware layout mapping block for the Cortex-M3 SysTick core peripheral registers.
//-----------------------------------------------------------------------------
struct SysTickMemMap
{
    // Control and Status Register.
    volatile uint32_t ControlStatus;

    // Reload Value Register.
    volatile uint32_t ReloadValue;

    // Current Value Register.
    volatile uint32_t CurrentValue;

    // Calibration Register.
    volatile uint32_t Calibration;
};

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Statics
//-----------------------------------------------------------------------------

// Global rolling counter tracking total clock ticks since processor power-on.
static volatile TimerTickType SystemTickCount = 0;

// Memory pointer targeted directly onto physical peripheral baseline registers.
static SysTickMemMap *const SysTickPeripheral = (SysTickMemMap *)SysTickBaseAddr;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Configures and enables the hardware internal SysTick exception framework.
//-----------------------------------------------------------------------------
void TargetSysTickInit()
{
    // Disable SysTick during setup.
    SysTickPeripheral->ControlStatus = 0;

    // Set reload for 1ms intervals.
    SysTickPeripheral->ReloadValue = SysTickReload1Ms;

    // Clear current counter value.
    SysTickPeripheral->CurrentValue = 0;

    // Enable counter, interrupt, and processor clock source.
    SysTickPeripheral->ControlStatus = (SysTickEnable | SysTickTickInt | SysTickClkSource);
}

//-----------------------------------------------------------------------------
// Centralized hardware initialization entry target execution point.
//-----------------------------------------------------------------------------
void TargetInit() { TargetSysTickInit(); }

//-----------------------------------------------------------------------------
// Read-interface query layer pulling the safe static runtime clock value.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount(void) { return SystemTickCount; }

//-----------------------------------------------------------------------------
// This target-specific wrapper handles the calling-convention compensation
// math and returns the clean stack pointer (SP) value of the current task.
//-----------------------------------------------------------------------------
StackType *GetTaskStackPointer()
{
    // Query the raw, unadjusted hardware stack pointer value from assembly.
    uintptr_t rawSp = GetStackPointer();

    // Return the raw address directly as no stack compensation is needed on ARM32.
    return (StackType *)rawSp;
}

//-----------------------------------------------------------------------------
// Classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sets up initial execution frames on the target process stack layout.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack()
{
    // Local stack tracing tracking pointer.
    StackType *sp = (StackType *)((uintptr_t)StackTop & ~7U);

    *(--sp) = (StackType)&TrampolineToNewTask;

    *(--sp) = 11;

    *(--sp) = 10;

    *(--sp) = 9;

    *(--sp) = 8;

    *(--sp) = 7;

    *(--sp) = 6;

    *(--sp) = 5;

    *(--sp) = 4;

    SavedSp = (StackType *)sp;
}

//-----------------------------------------------------------------------------
// extern "C"
//-----------------------------------------------------------------------------
extern "C"
{
    //-----------------------------------------------------------------------------
    // Periodic clock hardware interrupt execution trap called directly by Target.s.
    //-----------------------------------------------------------------------------
    void TimerTickIsr(void) { SystemTickCount++; }

    //-----------------------------------------------------------------------------
    // Mandatory POSIX low-level system compliance stubs satisfying Newlib linkages.
    //-----------------------------------------------------------------------------
    int _kill(int pid, int sig)
    {
        (void)pid;
        (void)sig;
        return -1;
    }

    int _getpid(void) { return 1; }

    caddr_t _sbrk(int incr)
    {
        (void)incr;
        return (caddr_t)0;
    }

    int _close(int file)
    {
        (void)file;
        return -1;
    }

    int _lseek(int file, int ptr, int dir)
    {
        (void)file;
        (void)ptr;
        (void)dir;
        return -1;
    }

    int _read(int file, char *ptr, int len)
    {
        (void)file;
        (void)ptr;
        (void)len;
        return -1;
    }

    int _write(int file, char *ptr, int len)
    {
        (void)file;
        (void)ptr;
        (void)len;
        return -1;
    }

} // extern "C"

} // namespace TicsNameSpace
