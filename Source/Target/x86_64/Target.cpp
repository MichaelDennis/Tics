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

    // 2. Force the starting address to a multiple of 16 per 64-bit ABI rules
    rawSp &= SixteenByteBoundaryMask;
    StackTop = (StackType *) rawSp;
    StackType *sp = (StackType *) rawSp;

    // 3. System V AMD64 Alignment Padding slot
    // Keeps the stack pointer balanced on a 16-byte boundary 
    *(--sp) = 0; 

    // 4. Push the ErrorHandler address using width-safe casting
    *(--sp) = (StackType) (uintptr_t) TrampolineToErrorHandler; 

    // 5. Push the actual starting STATIC function execution vector
    *(--sp) = (StackType) (uintptr_t) TrampolineToNewTask;

    // 6. Push fake preserved callee registers matching the 6 64-bit assembly 'popq' steps
    *(--sp) = 0; // Fake %rbp (terminates call stack trace loops cleanly)
    *(--sp) = 1; // Fake %rbx
    *(--sp) = 2; // Fake %r12
    *(--sp) = 3; // Fake %r13
    *(--sp) = 4; // Fake %r14
    *(--sp) = 5; // Fake %r15
    
    // Save the finalized stack pointer position back for scheduler allocation
    SavedSp = sp;
    return;
}

} // namespace TicsNameSpace
