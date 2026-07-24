/*
ARM Pure 64-bit (AArch64) Target.cpp

Under pure 64-bit ARM rules, parameters are passed natively via the 64-bit x0-x7 registers. 
The architecture enforces an absolute 16-byte stack pointer alignment whenever memory is 
accessed. To respect this hardware requirement, registers are pushed and popped in clean 
64-bit register pairs (stp and ldp), and your PrimeStack math reflects this layout perfectly.

Linux compile and link cmd line

First, install the arm 64 compiler by executing the cmd below:

sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

To manually compile and link your decoupled framework for a 64-bit ARM architecture 
(such as an ARMv8-A profile running a bare-metal environment), execute this command
 in your terminal from the Tics folder:

aarch64-linux-gnu-g++ -I./Source -g -march=armv8-a -O0 ./Source/Tics.cpp ./Source/Target/Arm64/Target.cpp ./Source/Target/Arm64/Target.s ./Examples/Hello.cpp -o ./Bin/Hello_Arm64.elf --specs=nosys.specs -Wl,--no-warnings

*/

#include "Tics.hpp" 
#include <cstdint>

extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();

namespace TicsNameSpace {

// Pure 64-bit ARM system timer tracking
volatile TimerTickType Tics_Arm64TimerTicks = 0;

TimerTickType GetSystemTickCount() {
    return (TimerTickType) Tics_Arm64TimerTicks;
}

void StackClass::PrimeStack() {
    // 1. Start at the absolute top of the allocated stack memory
    StackType rawSp = (StackType) StackTop;

    // 2. Force the starting address to a multiple of 16 per strict 64-bit hardware alignment rules
    rawSp &= SixteenByteBoundaryMask;
    StackTop = (StackType *) rawSp;
    StackType *sp = (StackType *) rawSp;

    // 3. Push execution vectors matching the 16-byte aligned register pairs
    // The top-level pair captures the execution targets (x30 = Link Register, x29 = Frame Pointer)
    *(--sp) = (StackType) (uintptr_t) TrampolineToNewTask;      // x30 (lr)
    *(--sp) = (StackType) (uintptr_t) TrampolineToErrorHandler;  // x29 (fp)
    
    // 4. Push fake preserved 64-bit callee registers matching the assembly layout pairs
    *(--sp) = 28; // x28
    *(--sp) = 27; // x27
    *(--sp) = 26; // x26
    *(--sp) = 25; // x25
    *(--sp) = 24; // x24
    *(--sp) = 23; // x23
    *(--sp) = 22; // x22
    *(--sp) = 21; // x21
    *(--sp) = 20; // x20
    *(--sp) = 19; // x19

    // Save the finalized stack pointer position back for scheduler allocation
    SavedSp = sp;
    return;
}

} // namespace TicsNameSpace


