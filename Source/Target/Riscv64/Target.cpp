/*
RISC-V 64-bit (RV64)Under the 64-bit RISC-V calling convention, registers expand to a full 8 bytes (64 bits). Pointers and context-tracking indices grow to match the expanded bus width. We swap out the 32-bit word assembly instructions (sw/lw) for the native 64-bit double-word instructions (sd/ld) to cleanly preserve the 14 core registers.1. The Cross-Compiler Command LineTo compile and link your decoupled framework for a RISC-V 64-bit target using the terminal, run this command:

riscv64-unknown-elf-g++ -I./Source -g -march=rv64imac -mabi=lp64 -O0 ./Source/Tics.cpp ./Source/Target/Riscv64/Target.cpp ./Source/Target/Riscv64/Target.s ./Examples/Hello.cpp -o ./Bin/Hello_Riscv64.elf

*/
#include "Tics.hpp" 
#include <cstdint>

// Memory-Mapped CLINT Base Address register tracking for RV64 mtime counter
#define RV64_CLINT_MTIME_ADDR (*(volatile uint64_t*)0x02004000)

extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();

namespace TicsNameSpace {

TimerTickType GetSystemTickCount() {
    return RV64_CLINT_MTIME_ADDR;
}

void StackClass::PrimeStack() {
    // 1. Start at the absolute top of the allocated stack memory
    uintptr_t rawSp = (uintptr_t) StackTop;

    // 2. Force the starting address to a multiple of 16 per RISC-V ABI rules
    rawSp &= SixteenByteBoundaryMask;
    StackTop = (StackType *) rawSp;
    StackType *sp = (StackType *) rawSp;

    // 3. Push fake preserved registers matching the 14 assembly slot locations (14 * 8 bytes)
    // We populate the positions sequentially to match the offsets loaded by Target.s
    *(--sp) = 11; // Fake s11
    *(--sp) = 10; // Fake s10
    *(--sp) = 9;  // Fake s9
    *(--sp) = 8;  // Fake s8
    *(--sp) = 7;  // Fake s7
    *(--sp) = 6;  // Fake s6
    *(--sp) = 5;  // Fake s5
    *(--sp) = 4;  // Fake s4
    *(--sp) = 3;  // Fake s3
    *(--sp) = 2;  // Fake s2
    *(--sp) = 1;  // Fake s1
    *(--sp) = 0;  // Fake s0 / Frame Pointer
    
    // Loaded into the Return Address register (ra) during context bootup to catch task termination
    *(--sp) = (StackType) (uintptr_t) TrampolineToErrorHandler; 
    
    // The initial task execution target address
    *(--sp) = (StackType) (uintptr_t) TrampolineToNewTask;      

    // Save the finalized stack pointer position back for scheduler allocation
    SavedSp = sp;
    return;
}

} // namespace TicsNameSpace
