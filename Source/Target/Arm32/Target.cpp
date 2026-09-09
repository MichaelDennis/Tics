#include "Tics.hpp"
#include <stdint.h>
#include <sys/types.h>

// ARM Cortex-M3 SysTick Peripheral Memory-Mapped Registers
#define SCS_BASE_ADDR (0xE000E000UL)
#define SYSTICK_BASE_ADDR (SCS_BASE_ADDR + 0x0010UL)

typedef struct
{
    volatile uint32_t CTRL;  // Control and Status Register
    volatile uint32_t LOAD;  // Reload Value Register
    volatile uint32_t VAL;   // Current Value Register
    volatile uint32_t CALIB; // Calibration Register
} SysTick_MemMap_t;

#define SysTick_Peripheral ((SysTick_MemMap_t *)SYSTICK_BASE_ADDR)

// SysTick Control Register bit masks
#define SYSTICK_ENABLE (1UL << 0)
#define SYSTICK_TICKINT (1UL << 1)
#define SYSTICK_CLKSOURCE (1UL << 2)

// Assuming 25 MHz CPU clock for QEMU mps2-an385 (25,000,000 / 1000 = 25,000 ticks per ms)
#define CPU_CLOCK_HZ 25000000UL
#define SYSTICK_RELOAD_1MS ((CPU_CLOCK_HZ / 1000UL) - 1UL)

// System tick counter state
static volatile TimerTickType g_systemTickCount = 0;

extern "C"
{
    // C Externals expected by Tics core and Target.s
    void TimerTickIsr(void) { g_systemTickCount++; }

    TimerTickType GetSystemTickCount(void) { return g_systemTickCount; }

    // POSIX Stubs for newlib (warning-free overrides)
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
        return (caddr_t)0; // Signal out-of-memory: Tics is static-only
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

    // Hardware Tick Initialization Function
    void Target_SysTick_Init(void)
    {
        SysTick_Peripheral->CTRL = 0;                  // Disable SysTick during setup
        SysTick_Peripheral->LOAD = SYSTICK_RELOAD_1MS; // Set reload for 1ms intervals
        SysTick_Peripheral->VAL = 0;                   // Clear current counter value
        SysTick_Peripheral->CTRL =
            SYSTICK_ENABLE | SYSTICK_TICKINT |
            SYSTICK_CLKSOURCE; // Enable counter, interrupt, and processor clock source
    }
}

namespace TicsNameSpace
{

void StackClass::PrimeStack()
{
    StackType *sp = (StackType *)((StackType)StackTop & ~7U);

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
    return;
}

} // namespace TicsNameSpace
