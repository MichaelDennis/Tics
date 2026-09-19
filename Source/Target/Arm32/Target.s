/*
MIT License

Copyright (c) 2026 Michael Dennis McDonnell (Tics Realtime)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//-----------------------------------------------------------------------------
// Copyright (c) 2026, Tics Realtime (Michael Dennis McDonnell)
// Target: ARM Cortex-M (Thumb-2) Cooperative Context Switch & Vectors
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Core assembler directives configuring target platform characteristics.
//-----------------------------------------------------------------------------

// Use modern unified ARM/Thumb assembly syntax.
.syntax unified

// Target the Cortex-M3 core instruction set.
.cpu cortex-m3

// Generate 16/32-bit Thumb-2 instructions.
.thumb

//-----------------------------------------------------------------------------
// External Linker Symbols
//-----------------------------------------------------------------------------

// The top boundary memory address of the Main Stack Pointer (MSP).
.extern _estack

// The starting RAM address boundary for uninitialized global/static variables.
.extern __bss_start__

// The ending RAM address boundary for uninitialized global/static variables.
.extern __bss_end__

// The starting RAM address destination for initialized variables.
.extern _sdata

// The ending RAM address destination for initialized variables.
.extern _edata

// The source Flash memory load address for initialized variables.
.extern _sidata

// The starting address of the global C++ constructors function pointer array.
.extern __init_array_start

// The ending address boundary of the global constructor function pointer array.
.extern __init_array_end

// The primary execution entry point function for user application code.
.extern main

// The external core kernel C++ timer increment handler routine.
.extern TimerTickIsr

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

// Export the Reset Handler hardware boot symbol.
.global Reset_Handler

// Export the Default Handler exception catch symbol.
.global Default_Handler

// Export the SysTick Handler periodic timer symbol.
.global SysTick_Handler

// Export the Task Switch cooperative scheduling symbol.
.global TaskSwitch

// Export the Get Stack Pointer diagnostics symbol.
.global GetStackPointer

//-----------------------------------------------------------------------------
// Statics
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Vector Table Section (.isr_vector)
//-----------------------------------------------------------------------------
.section .isr_vector, "a", %progbits

//-----------------------------------------------------------------------------
// Vector Attributes
//-----------------------------------------------------------------------------

// Declare g_pfnVectors as an object type symbol identifier.
.type g_pfnVectors, %object

// Calculate the absolute size footprint of the vector object array.
.size g_pfnVectors, .-g_pfnVectors

//-----------------------------------------------------------------------------
// Vector Table
//-----------------------------------------------------------------------------

// The primary hardware exception vector table configuration mapping table.
g_pfnVectors:
    // 0x00: Initial Main Stack Pointer (MSP) top boundary.
    .word _estack

    // 0x04: Reset and CPU hardware boot configuration entry point.
    .word Reset_Handler

    // 0x08: Non-Maskable Interrupt (NMI) execution exception handler.
    .word Default_Handler

    // 0x0C: Hard Fault catastrophic error recovery trap vector.
    .word Default_Handler

    // 0x10: Reserved vector channel slot.
    .word 0

    // 0x14: Reserved vector channel slot.
    .word 0

    // 0x18: Reserved vector channel slot.
    .word 0

    // 0x1C: Reserved vector channel slot.
    .word 0

    // 0x20: Reserved vector channel slot.
    .word 0

    // 0x24: Reserved vector channel slot.
    .word 0

    // 0x28: Reserved vector channel slot.
    .word 0

    // 0x2C: SVCall core kernel runtime exception driver handler.
    .word Default_Handler

    // 0x30: Reserved vector channel slot.
    .word 0

    // 0x34: Reserved vector channel slot.
    .word 0

    // 0x38: PendSV context-switching transaction scheduling vector.
    .word Default_Handler

    // 0x3C: SysTick peripheral system core interval periodic exception timer handler.
    .word SysTick_Handler

//-----------------------------------------------------------------------------
// Code Segment (.text)
//-----------------------------------------------------------------------------
.text

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Reset Handler: Explicit BSS/Data Initialization and Boot Handoff.
//
// Description:
//   The hardware boot entry sequence executing immediately on processor 
//   power-on. This routine zero-initializes the uninitialized RAM (.bss) 
//   segment, copies initialized global variables (.data) from Flash into RAM, 
//   manually runs global C++ constructors via the compile-time constructor 
//   table (.init_array), and hands off control to the main C++ application.
//-----------------------------------------------------------------------------
.thumb_func
Reset_Handler:
    // Load the starting address of the .bss uninitialized memory section.
    ldr r0, =__bss_start__

    // Load the ending address boundary of the .bss memory section.
    ldr r1, =__bss_end__

    // Set register r2 to zero to prepare for clearing memory.
    movs r2, #0

.bss_loop:
    // Compare the current memory pointer against the end boundary pointer.
    cmp r0, r1

    // Branch out of the loop if the current pointer has reached the end.
    bge .bss_done

    // Store zero into the current memory address and increment the address by 4.
    str r2, [r0], #4

    // Jump back to the beginning of the loop to process the next word.
    b   .bss_loop

.bss_done:
    // Load the starting RAM target destination address for the .data section.
    ldr r0, =_sdata

    // Load the ending RAM boundary address for the .data section.
    ldr r1, =_edata

    // Load the source Flash memory address where data is currently stored.
    ldr r2, =_sidata

.data_loop:
    // Compare the destination pointer against its ending boundary.
    cmp r0, r1

    // Branch out of the loop if all initialized variables are copied.
    bge .data_done

    // Read a 4-byte initialized value from Flash memory and advance the pointer.
    ldr r3, [r2], #4

    // Write that 4-byte value into RAM destination memory and advance the pointer.
    str r3, [r0], #4

    // Jump back to the loop entry to copy the next initialized variable.
    b   .data_loop

.data_done:
    // Load the starting address of the global C++ constructors function table.
    ldr r0, =__init_array_start

    // Load the ending address boundary of the constructor function table.
    ldr r1, =__init_array_end

.ctor_loop:
    // Compare the constructor pointer against the ending table boundary.
    cmp r0, r1

    // Branch out of the loop if all global constructor routines have executed.
    bge .ctor_done

    // Load the actual constructor function pointer and advance the table address.
    ldr r2, [r0], #4

    // Check if the loaded function pointer address evaluates to zero.
    cmp r2, #0

    // Skip execution and advance if the loaded pointer is null.
    beq .ctor_next

    // Push current loop pointers onto the stack to preserve them across the call.
    push {r0, r1}

    // Branch with link and switch state to execute the constructor function.
    blx r2

    // Pop the original loop pointers back from the stack to resume the loop.
    pop {r0, r1}

.ctor_next:
    // Jump back to the loop entry to process the next constructor entry.
    b   .ctor_loop

.ctor_done:
    // Hand off execution control to the main C++ application framework.
    bl  main

.dead_loop:
    // Loop indefinitely to trap execution if the main function ever exits.
    b   .dead_loop

//-----------------------------------------------------------------------------
// Default Handler: Catch-all trap for unexpected interrupts.
//
// Description:
//   The fallback error handler vector used to catch unconfigured hardware 
//   exceptions. Traps the processor in an infinite loop for diagnostics.
//-----------------------------------------------------------------------------
.thumb_func
Default_Handler:
    // Trap execution inside an infinite loop to catch the unhandled exception.
    b   Default_Handler

//-----------------------------------------------------------------------------
// SysTick Handler: 1ms system clock tick ISR.
//
// Description:
//   The hardware clock interrupt service routine responding to SysTick 
//   register overflows. Saves volatile context and bridges to the C++ kernel.
//-----------------------------------------------------------------------------
.thumb_func
SysTick_Handler:
    // Save the Link Register value onto the stack to track our return path.
    push {lr}

    // Call the external C++ function tracking periodic kernel uptime ticks.
    bl  TimerTickIsr

    // Pop the saved address straight into the Program Counter to exit the ISR.
    pop {pc}

//-----------------------------------------------------------------------------
// Task Switch: Cooperative context switch routine.
//
// Description:
//   Switches CPU context execution frames between two active kernel threads.
//   Saves the current callee-saved registers and swaps core stack pointers.
//
// Arguments:
//   r0 - currentTaskSavedSp: Pointer to save location for current task SP.
//   r1 - newTaskSavedSp: Raw stack address of the incoming task.
//-----------------------------------------------------------------------------
.thumb_func
TaskSwitch:
    // Push the current core CPU registers and link register onto the task stack.
    push {r4-r11, lr}

    // Save the active CPU Stack Pointer address into the old task structure.
    str  sp, [r0]

    // Assign the new task's stack pointer directly into the active CPU register.
    mov  sp, r1

    // Pop the new task's saved register frame straight into active execution.
    pop  {r4-r11, pc}

//-----------------------------------------------------------------------------
// Get Stack Pointer: Returns current SP.
//
// Description:
//   Query routine pulling the active address layer of the hardware stack 
//   pointer register for diagnostic validation checks.
//
// Return Value:
//   r0 - The current value of the CPU stack pointer register.
//-----------------------------------------------------------------------------
.thumb_func
GetStackPointer:
    // Move the active Stack Pointer address into register r0 for the return value.
    mov  r0, sp

    // Return to the calling C++ function using the link register destination.
    bx   lr

    .end
    