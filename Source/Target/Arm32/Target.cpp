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
// Copyright (c) 2026, Tics Realtime (Michael Dennis McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <stdint.h>
#include <sys/types.h>

//-----------------------------------------------------------------------------
// Start TicsNameSpace
//-----------------------------------------------------------------------------
namespace TicsNameSpace
{

//-----------------------------------------------------------------------------
// Hardware Layout Enums
//-----------------------------------------------------------------------------
enum SysTickPeripheralEnum : uint32_t
{
    ScsBaseAddr = 0xE000E000UL,
    SystickBaseAddr = (ScsBaseAddr + 0x0010UL),

    // Control Register bit masks
    SystickEnable = (1UL << 0),
    SystickTickint = (1UL << 1),
    SystickClksource = (1UL << 2),

    // Hardware Frequency Conversions (25 MHz CPU clock for QEMU mps2-an385)
    CpuClockHz = 25000000UL,
    SystickReload1Ms = ((CpuClockHz / 1000UL) - 1UL),
};

//-----------------------------------------------------------------------------
// Hardware Register Structure Map
//-----------------------------------------------------------------------------
struct SysTick_MemMap_t
{
    volatile uint32_t CTRL;  // Control and Status Register
    volatile uint32_t LOAD;  // Reload Value Register
    volatile uint32_t VAL;   // Current Value Register
    volatile uint32_t CALIB; // Calibration Register
};

// Pointer mapping directly onto physical microcontroller register memory
static SysTick_MemMap_t *const SysTick_Peripheral =
    reinterpret_cast<SysTick_MemMap_t *>(SystickBaseAddr);

//-----------------------------------------------------------------------------
// Global Target States
//-----------------------------------------------------------------------------
static volatile TimerTickType g_systemTickCount = 0;

//-----------------------------------------------------------------------------
// C++ Target Core Functions
//-----------------------------------------------------------------------------

void Target_SysTick_Init()
{
    SysTick_Peripheral->CTRL = 0;                // Disable SysTick during setup
    SysTick_Peripheral->LOAD = SystickReload1Ms; // Set reload for 1ms intervals
    SysTick_Peripheral->VAL = 0;                 // Clear current counter value
    SysTick_Peripheral->CTRL = (SystickEnable | SystickTickint | SystickClksource);
}

void TargetInit() { Target_SysTick_Init(); }

TimerTickType GetSystemTickCount(void) { return g_systemTickCount; }

//-----------------------------------------------------------------------------
/// \brief StackClass::PrimeStack
///
/// Sets up initial execution frames on the target process stack layout.
//-----------------------------------------------------------------------------
void StackClass::PrimeStack()
{
    StackType *sp = reinterpret_cast<StackType *>(reinterpret_cast<StackType>(StackTop) & ~7U);

    *(--sp) = reinterpret_cast<StackType>(&TrampolineToNewTask);
    *(--sp) = 11;
    *(--sp) = 10;
    *(--sp) = 9;
    *(--sp) = 8;
    *(--sp) = 7;
    *(--sp) = 6;
    *(--sp) = 5;
    *(--sp) = 4;

    SavedSp = reinterpret_cast<StackType *>(sp);
}

//-----------------------------------------------------------------------------
// Explicit ISR Interface Hooks (Accessed by Target.s)
//-----------------------------------------------------------------------------
extern "C"
{
    void TimerTickIsr(void) { g_systemTickCount++; }
}

//-----------------------------------------------------------------------------
// Newlib POSIX System Call Stubs (Satisfying Standard Library Linkage)
//-----------------------------------------------------------------------------
extern "C"
{
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
        return reinterpret_cast<caddr_t>(0);
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
}

} // namespace TicsNameSpace
