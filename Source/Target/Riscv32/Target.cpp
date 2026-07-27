/*
 Hello World compile line 32 bit execute in Tics folder: riscv64-unknown-elf-g++ --specs=picolibc.specs -I/home/mdm/projects/Tics/Source -g -march=rv32imac -mabi=ilp32 -fno-exceptions /home/mdm/projects/Tics/Source/Target/Riscv32/Main.cpp -O0 /home/mdm/projects/Tics/Source/Tics.cpp /home/mdm/projects/Tics/Source/Target/Riscv32/Target.cpp /home/mdm/projects/Tics/Source/Target/Riscv32/Target.s -o /home/mdm/projects/Tics/Bin/Main.elf

 Hello World compile line 64 bit execute in Tics folder: riscv64-unknown-elf-g++ -specs=picolibc.specs Sandbox/hello.cpp -o Sandbox/hello_64.elf

*/
#include "Tics.hpp" 
#include <stdlib.h>

#define RV32_CLINT_MTIME_ADDR (*(volatile uint64_t*)0x02004000)

extern "C" void TrampolineToErrorHandler();
extern "C" void TrampolineToNewTask();

namespace TicsNameSpace {

TimerTickType GetSystemTickCount() {
    return (TimerTickType) RV32_CLINT_MTIME_ADDR;
}

void StackClass::PrimeStack() {
    StackType rawSp = (StackType) StackTop;
    rawSp &= SixteenByteBoundaryMask;
    StackTop = (StackType *) rawSp;
    StackType *sp = (StackType *) rawSp;

    *(--sp) = 11; 
    *(--sp) = 10; 
    *(--sp) = 9;  
    *(--sp) = 8;  
    *(--sp) = 7;  
    *(--sp) = 6;  
    *(--sp) = 5;  
    *(--sp) = 4;  
    *(--sp) = 3;  
    *(--sp) = 2;  
    *(--sp) = 1;  
    *(--sp) = 0;  
    
    *(--sp) = (StackType) (uintptr_t) TrampolineToErrorHandler; 
    *(--sp) = (StackType) (uintptr_t) TrampolineToNewTask;      

    SavedSp = sp;
    return;
}

} // namespace TicsNameSpace
