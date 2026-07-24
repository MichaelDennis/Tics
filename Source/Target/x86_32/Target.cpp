#include "Tics.hpp" 
#include <stdlib.h>
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
    StackType rawSp;
    rawSp = (StackType) StackTop;
    rawSp &= SixteenByteBoundaryMask;
    StackTop = (StackType *) rawSp;
    StackType *sp = (StackType *) rawSp;

    *(--sp) = 0; 
    *(--sp) = 0;
    *(--sp) = (StackType) (uintptr_t) TrampolineToErrorHandler; 
    *(--sp) = (StackType) (uintptr_t) TrampolineToNewTask;
    *(--sp) = 0; 
    *(--sp) = 1; 
    *(--sp) = 2; 
    *(--sp) = 3; 
    
    SavedSp = sp;
    return;
}

} // namespace TicsNameSpace
