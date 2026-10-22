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
/// This file provides the low-level ARM32 Cortex-M3 target specific hardware
/// initializations, timer tracking drivers, and required system stub functions
/// for the Tics RTOS engine.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <stdint.h>
#include <sys/types.h>

//-----------------------------------------------------------------------------
/// extern "C"
//-----------------------------------------------------------------------------
extern "C"
{
    // This function returns the current value of the stack pointer register.
    uintptr_t GetStackPointer();
}

//-----------------------------------------------------------------------------
/// Start TicsNameSpace
//-----------------------------------------------------------------------------
namespace TicsNameSpace
{

//-----------------------------------------------------------------------------
/// Enums
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Core register constants and hardware clock metrics for the SysTick module.
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
/// Structs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Hardware layout mapping block for the Cortex-M3 SysTick core peripheral registers.
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
/// Globals
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Statics
//-----------------------------------------------------------------------------

/// Global rolling counter tracking total clock ticks since processor power-on.
static volatile TimerTickType SystemTickCount = 0;

/// Memory pointer targeted directly onto physical peripheral baseline registers.
static SysTickMemMap *const SysTickPeripheral = (SysTickMemMap *)SysTickBaseAddr;

//-----------------------------------------------------------------------------
/// Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \brief Arm timer isr setup.
///
/// The Tics required 1 ms timer tick is realized on the Arm
/// by setting up the Arm countdown timer to count to 0 and reset
/// every 1 ms. The setup required to do this is given below.
//-----------------------------------------------------------------------------
void TargetSysTickInit()
{
    /// Disable SysTick during setup.
    SysTickPeripheral->ControlStatus = 0;

    /// Set reload for 1ms intervals.
    SysTickPeripheral->ReloadValue = SysTickReload1Ms;

    /// Clear current counter value.
    SysTickPeripheral->CurrentValue = 0;

    /// Enable counter, interrupt, and processor clock source.
    SysTickPeripheral->ControlStatus = (SysTickEnable | SysTickTickInt | SysTickClkSource);
}

//-----------------------------------------------------------------------------
/// \brief This function provides a Tics compliant target init function.
///
/// Tics requires that a function as named below be provided so that
/// Tics can call it at startup time to initialize the hardware system.
//-----------------------------------------------------------------------------
void TargetInit() { TargetSysTickInit(); }

//-----------------------------------------------------------------------------
/// \brief This function provides a Tics compliant function to return
/// the system 1 ms tick count.
//-----------------------------------------------------------------------------
TimerTickType GetSystemTickCount(void) { return SystemTickCount; }

//-----------------------------------------------------------------------------
/// \brief This function returns the value of the stack pointer register.
//-----------------------------------------------------------------------------
StackType *GetTaskStackPointer()
{
    /// Query the raw, unadjusted hardware stack pointer value from assembly.
    uintptr_t rawSp = GetStackPointer();

    /// Return the raw address directly as no stack compensation is needed on ARM32.
    return (StackType *)rawSp;
}

//-----------------------------------------------------------------------------
/// \brief the following are class functions required by Tics.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \brief This function primes the stack.
///
/// The first time a task is run it requires that the stack be
/// pre-populated so that the context switching code can pop the
/// register values off the stack. The contents of the registers on
/// startup is immaterial - an values can be used.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack()
{
    /// Local stack tracing tracking pointer.
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
/// extern "C"
//-----------------------------------------------------------------------------
extern "C"
{
    //-----------------------------------------------------------------------------
    ///\brief This function updates the 1 ms system tick counter variable.
    ///
    /// SystemTickCount is a static variable that contains the current 1 ms
    /// tick count value, and is returned by the function GetSystemTickCount
    /// which is called to Tics.
    //-----------------------------------------------------------------------------
    void TimerTickIsr(void) { SystemTickCount++; }

    //-----------------------------------------------------------------------------
    ///\brief  Mandatory POSIX low-level system compliance stubs
    ///
    /// Even though we are not using the functions listed below, we need to
    /// define dummy versions to satisfy the linker, otherwise we will get
    /// unresolved references when we try to link.
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

} /// extern "C"

} // namespace TicsNameSpace
