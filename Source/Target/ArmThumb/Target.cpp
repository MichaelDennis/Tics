//-----------------------------------------------------------------------------
// Cmd line build in Tics folder:  arm-none-eabi-g++ -I./Source -g -mcpu=cortex-m4 -mthumb -O0 ./Source/Tics.cpp ./Source/Target/ArmThumb/Target.cpp ./Source/Target/ArmThumb/Target.s ./Examples/Hello.cpp -o ./Bin/Hello.elf --specs=nosys.specs -Wl,--no-warnings
//-----------------------------------------------------------------------------

#include "Tics.hpp" 
#include <cstdint>

extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();

namespace TicsNameSpace {

// Hardware variable updated upward via a 1ms SysTick interrupt loop
volatile TimerTickType Tics_MillisecondCount = 0;

TimerTickType GetSystemTickCount() {
    return (TimerTickType) Tics_MillisecondCount;
}

void StackClass::PrimeStack() {
    // 1. Start at the absolute top of the allocated stack memory
    uintptr_t rawSp = (uintptr_t) StackTop;

    // 2. Force the starting address to a multiple of 16 per AAPCS alignment rules
    rawSp &= SixteenByteBoundaryMask;
    StackTop = (StackType *) rawSp;
    StackType *sp = (StackType *) rawSp;

    // 3. First item popped directly into pc/lr during context restore
    *(--sp) = (StackType) (uintptr_t) TrampolineToNewTask;      
    
    // 4. Becomes the active Link Register (lr) to catch task termination loops
    *(--sp) = (StackType) (uintptr_t) TrampolineToErrorHandler;  
    
    // 5. Push fake preserved registers matching the 8 Core Thumb 'pop' steps (r4-r11)
    *(--sp) = 11; // Fake r11
    *(--sp) = 10; // Fake r10
    *(--sp) = 9;  // Fake r9
    *(--sp) = 8;  // Fake r8
    *(--sp) = 7;  // Fake r7
    *(--sp) = 6;  // Fake r6
    *(--sp) = 5;  // Fake r5
    *(--sp) = 4;  // Fake r4

    // Save the finalized stack pointer position back for scheduler allocation
    SavedSp = sp;
    return;
}

} // namespace TicsNameSpace
