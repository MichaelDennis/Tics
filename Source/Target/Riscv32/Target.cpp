/*
 Hello World compile line 32 bit execute in Tics folder: riscv64-unknown-elf-g++ --specs=picolibc.specs -I/home/mdm/projects/Tics/Source -g -march=rv32imac -mabi=ilp32 -fno-exceptions /home/mdm/projects/Tics/Source/Target/Riscv32/Main.cpp -O0 /home/mdm/projects/Tics/Source/Tics.cpp /home/mdm/projects/Tics/Source/Target/Riscv32/Target.cpp /home/mdm/projects/Tics/Source/Target/Riscv32/Target.s -o /home/mdm/projects/Tics/Bin/Main.elf

 Hello World compile line 64 bit execute in Tics folder: riscv64-unknown-elf-g++ -specs=picolibc.specs Sandbox/hello.cpp -o Sandbox/hello_64.elf

 Build: riscv64-unknown-elf-g++ -march=rv32imac -mabi=ilp32 -g -O0 --specs=picolibc.specs --oslib=semihost -I./Source ./Source/Target/Riscv32/Target.s ./Source/Target/Riscv32/Target.cpp ./Source/Tics.cpp ./Source/Target/Riscv32/Main.cpp -o ./Bin/Main.elf

 QEMU: qemu-system-riscv32 -machine virt -cpu rv32 -smp 1 -m 128M -bios none -kernel ./Bin/Main.elf -display none -s -S &clear

Terminate: killall qemu-system-riscv32

Do this to run:

0. Terminate tasks (see above)
1. Debug and Run panel select drop down value of: Connect GDB to QEMU (Tics)
2. Open Target/Riscv32/Main.cpp
3. Run QEMU (see above)
3. Press F5

*/
#include "Tics.hpp" 
#include <stdlib.h>

#define RV32_CLINT_MTIME_ADDR (*(volatile uint32_t*)0x0200BFF8)

extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();

namespace TicsNameSpace {

static uint32_t lastRawHardwareTicks = 0;
static uint32_t subMsTicksBucket = 0;
static uint32_t freeRunningMsCounter = 0;

TimerTickType GetSystemTickCount() {
    // 1. Grab the absolute, ever-growing 10MHz hardware clock
    uint32_t currentRawHardwareTicks = RV32_CLINT_MTIME_ADDR;

    // Test counter.
    unsigned int counter = 0;

    // 2. Find the tiny slice of raw ticks that passed since the last poll
    uint32_t elapsedTicks = currentRawHardwareTicks - lastRawHardwareTicks;
    lastRawHardwareTicks = currentRawHardwareTicks;

    // 3. Drop them into our sub-millisecond remainder bucket
    subMsTicksBucket += elapsedTicks;

    // 4. Drain the bucket completely to catch every single millisecond
    while (subMsTicksBucket >= 10000) {
        freeRunningMsCounter++;
        subMsTicksBucket -= 10000;
        counter++;
    }

    // 5. Return the 32-bit millisecond integer (wraps back to 0 naturally)
    return (TimerTickType)freeRunningMsCounter;
}

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
    
    // RA (TrampolineToNewTask) is at the lowest memory address (Chronologically last push)
    *(--sp) = (uint32_t)(uintptr_t)TrampolineToNewTask; 

    // 3. Save the finalized stack pointer position back for scheduler allocation
    // sp is pointing EXACTLY at the memory slot holding TrampolineToNewTask!
    SavedSp = (StackType *) sp;
    return;
}

} // namespace TicsNameSpace
