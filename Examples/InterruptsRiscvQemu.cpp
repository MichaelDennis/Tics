//-----------------------------------------------------------------------------
// Copyright (c) 2026, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// QEMU Bare-Metal RISC-V 32-bit Hardware Interrupt Test
//-----------------------------------------------------------------------------
#include "Tics.hpp"

//-----------------------------------------------------------------------------
// QEMU RISC-V 'virt' Machine CLINT Hardware Registers (Memory-Mapped)
//-----------------------------------------------------------------------------
volatile uint64_t *const CLINT_MTIME = reinterpret_cast<uint64_t *>(0x0200BFF8);
volatile uint64_t *const CLINT_MTIMECMP = reinterpret_cast<uint64_t *>(0x02004000);

// Hardware execution tick interval (Adjust to alter frequency)
const uint64_t TICK_INTERVAL = 10000000;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
extern "C" void m_timer_isr(void) __attribute__((interrupt));
void SetupQemuTimerInterrupt();

//-----------------------------------------------------------------------------
// This class defines the data object that will be sent from the isr to the task.
//-----------------------------------------------------------------------------
class IsrDataClass
{
  public:
    int A;
    int B;
    int C;

    IsrDataClass(int a = 13, int b = 14, int c = 15) : A(a), B(b), C(c) {}
};

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
FifoClass *IsrFifo;
int InterruptCount = 0;

class TaskBClass : public TaskClass
{
  public:
    TaskBClass(const char *name = 0) : TaskClass(name) {}
    void Task();
};

TaskBClass *TaskB;

//-----------------------------------------------------------------------------
// Assembly Trampoline Hook
// Intercepts the low-level machine trap and forces a branch directly to our C++.
//-----------------------------------------------------------------------------
void __attribute__((naked, aligned(4))) TestTrapVector() { asm volatile("j m_timer_isr\n"); }

//-----------------------------------------------------------------------------
// Initialize the RISC-V Machine-Level Core Interrupt Controller (CLINT)
//-----------------------------------------------------------------------------
void SetupQemuTimerInterrupt()
{
    // 1. Map the machine trap vector register to our test trampoline
    asm volatile("csrw mtvec, %0" ::"r"(TestTrapVector));

    // 2. Set the initial hardware timer compare target matching current ticks
    *CLINT_MTIMECMP = *CLINT_MTIME + TICK_INTERVAL;

    // 3. Enable Machine Timer Interrupts (Bit 7 in the mie register)
    asm volatile("csrs mie, %0" ::"r"(1 << 7));

    // 4. Globally enable core hardware interrupts (Bit 3 in the mstatus register)
    asm volatile("csrs mstatus, %0" ::"r"(1 << 3));
}

//-----------------------------------------------------------------------------
// True Bare-Metal RISC-V Hardware ISR.
// Executed forcefully by the hardware CPU, completely outside the Tics context.
// All interrupts remain disabled during this execution loop.
//-----------------------------------------------------------------------------
extern "C" void m_timer_isr(void)
{
    static IsrDataClass isrData;

    // 1. Immediately schedule the next hardware timer flag threshold
    *CLINT_MTIMECMP = *CLINT_MTIME + TICK_INTERVAL;

    // 2. Simulate reading physical sensor register values
    isrData.A++;
    isrData.B++;
    isrData.C++;

    // 3. Forward the data block into the Tics deferred task queue safely
    Send(TaskB, IsrFifo, &isrData);

    // Hardware automatically restores task context on exit via mret!
}

//-----------------------------------------------------------------------------
// Implement the task that processes the isr data
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    IsrDataClass isrData;

    // Initialize the hardware timer interrupt controller directly inside the task loop
    SetupQemuTimerInterrupt();

    while (true)
    {
        // Stand suspended until the hardware ISR inserts data into the FIFO
        Wait(IsrFifo, &isrData);

        // Simply a place to set a breakpoint.
        InterruptCount++;

        // Under bare-metal QEMU, we can replace 'cout' with a custom UART print
        // function or drop a breakpoint here to verify 'isrData' increment values.
    }
}

//-----------------------------------------------------------------------------
// Main entry point
//-----------------------------------------------------------------------------
int main()
{
    int fifoSlotSize = sizeof(IsrDataClass);

    IsrFifo = new FifoClass(fifoSlotSize, 8);

    TaskB = new TaskBClass("TaskB");

    Suspend();

    return 0;
}
