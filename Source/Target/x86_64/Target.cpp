#include "Tics.hpp" 
#include <cstdint>
#include <time.h>

extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();
extern "C" int clock_gettime (clockid_t __clock_id, struct timespec *__tp);

namespace TicsNameSpace {

TimerTickType GetSystemTickCount() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (TimerTickType) tp.tv_sec * 1000 + (tp.tv_nsec / 1000000);
}

void StackClass::PrimeStack() {
    // 1. Start at the absolute top of the allocated stack memory
    uintptr_t rawSp = (uintptr_t) StackTop;

    // 2. Force the starting address to a multiple of 16 per RISC-V ABI rules
    rawSp &= SixteenByteBoundaryMask;
    
    // Use an explicit 32-bit pointer to ensure strict 4-byte word arithmetic
    uint32_t *sp = (uint32_t *) rawSp;

    // 3. Allocate a 56-byte frame (14 slots * 4 bytes) matching your TaskSwitch assembly
    sp -= 14;

    // 4. Populate the frame offsets to perfectly mirror your assembly instructions:
    
    // Offset 0(sp) -> Loaded into ra (This is the initial task execution vector!)
    sp[0] = (uint32_t) (uintptr_t) TrampolineToNewTask; 
    
    // Offsets 4(sp) through 48(sp) -> Loaded into registers s0 through s11
    sp[1]  = 0;  // Fake s0 / rbp (terminates debugger backtraces cleanly)
    sp[2]  = 1;  // Fake s1
    sp[3]  = 2;  // Fake s2
    sp[4]  = 3;  // Fake s3
    sp[5]  = 4;  // Fake s4
    sp[6]  = 5;  // Fake s5
    sp[7]  = 6;  // Fake s6
    sp[8]  = 7;  // Fake s7
    sp[9]  = 8;  // Fake s8
    sp[10] = 9;  // Fake s9
    sp[11] = 10; // Fake s10
    sp[12] = 11; // Fake s11
    
    // Offset 52(sp) -> Empty 4-byte alignment slot (keeps frame 16-byte boundary stable)
    sp[13] = 0; 

    // 5. Save the finalized stack pointer position back for scheduler allocation
    SavedSp = (StackType *) sp;
    return;
}

} // namespace TicsNameSpace
