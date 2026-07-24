//-----------------------------------------------------------------------------
// ARM 32 cmd line compile and link. Execute cmd from the Tics folder.
//
// arm-none-eabi-g++ -I./Source -g -march=armv7-a -marm -O0 ./Source/Tics.cpp ./Source/Target/Arm32/Target.cpp ./Source/Target/Arm32/Target.s ./Examples/Hello.cpp -o ./Bin/Hello_Arm32.elf --specs=nosys.specs -Wl,--no-warnings
//-----------------------------------------------------------------------------

#include "Tics.hpp" 
#include <cstdint>

extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();

namespace TicsNameSpace {

// Pure 32-bit ARM hardware timer variable tracking
volatile uint32_t Tics_Arm32TimerTicks = 0;

TimerTickType GetSystemTickCount() {
    return (TimerTickType) Tics_Arm32TimerTicks;
}

void StackClass::PrimeStack() {
    // 1. Start at the absolute top of the allocated stack memory
    StackType rawSp = (StackType) StackTop;

    // 2. Force the starting address to a multiple of 16 per AAPCS alignment rules
    rawSp &= SixteenByteBoundaryMask;
    StackTop = (StackType *) rawSp;
    StackType *sp = (StackType *) rawSp;

    // 3. Target execution vector popped straight into pc
    *(--sp) = (StackType) (uintptr_t) TrampolineToNewTask;      
    
    // 4. Becomes active Link Register (lr) for task termination catching
    *(--sp) = (StackType) (uintptr_t) TrampolineToErrorHandler;  
    
    // 5. Push fake preserved registers matching the 8 Core ARM 'ldmfd' steps (r4-r11)
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
