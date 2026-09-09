//-----------------------------------------------------------------------------
// Copyright (c) 2026, Tics Realtime (Michael Dennis McDonnell)
// Target: ARM Cortex-M (Thumb-2) Cooperative Context Switch & Vectors
//-----------------------------------------------------------------------------

.syntax unified    @ Use modern unified ARM/Thumb assembly syntax
.cpu cortex-m3     @ Target the Cortex-M3 core instruction set
.thumb             @ Generate 16/32-bit Thumb-2 instructions

//-----------------------------------------------------------------------------
// Vector Table Section (.isr_vector)
//
// The .section directive format in GNU as:
//   .section name [, "flags" [, %type [, @comment ]]]
// Common flags used here:
//   "a" = Allocatable (occupies memory in the output binary)
//   %progbits = Program data (contains actual binary instructions/data, not BSS)
//-----------------------------------------------------------------------------
.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object
.size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
    .word _estack              @ 0x00: Initial Main Stack Pointer (MSP) top
    .word Reset_Handler        @ 0x04: Reset / Boot Entry Point
    .word Default_Handler      @ 0x08: Non-Maskable Interrupt (NMI)
    .word Default_Handler      @ 0x0C: Hard Fault Handler
    .word 0                    @ 0x10: Reserved
    .word 0                    @ 0x14: Reserved
    .word 0                    @ 0x18: Reserved
    .word 0                    @ 0x1C: Reserved
    .word 0                    @ 0x20: Reserved
    .word 0                    @ 0x24: Reserved
    .word 0                    @ 0x28: Reserved
    .word Default_Handler      @ 0x2C: SVCall Handler
    .word 0                    @ 0x30: Reserved
    .word 0                    @ 0x34: Reserved
    .word Default_Handler      @ 0x38: PendSV Handler (Unused by Tics cooperative core)
    .word SysTick_Handler      @ 0x3C: SysTick Timer Handler (1ms clock tick)

//-----------------------------------------------------------------------------
// Code Segment (.text)
// Contains executable instructions placed in Flash memory.
//-----------------------------------------------------------------------------
.text

//-----------------------------------------------------------------------------
// Reset Handler: Boot entry point
//-----------------------------------------------------------------------------
.global Reset_Handler
.thumb_func
Reset_Handler:
    @ Hand off to the standard C/C++ runtime library entry point (_start).
    @ This initializes .data, zero-fills .bss, and runs all global C++ constructors.
    ldr     r0, =_start
    bx      r0

//-----------------------------------------------------------------------------
// Default Handler: Catch-all trap for unexpected interrupts
//-----------------------------------------------------------------------------
.global Default_Handler
.thumb_func
Default_Handler:
    b       Default_Handler    @ Infinite trap loop for debugging

//-----------------------------------------------------------------------------
// SysTick_Handler: 1ms system clock tick ISR
// Cortex-M hardware automatically pushes/pops basic registers (r0-r3, r12, lr, pc, xPSR).
//-----------------------------------------------------------------------------
.global SysTick_Handler
.thumb_func
SysTick_Handler:
    push    {lr}               @ Preserve link register before branching to C++
    bl      TimerTickIsr       @ Call external C-linkage tick function
    pop     {pc}               @ Return from interrupt

//-----------------------------------------------------------------------------
// void TaskSwitch(void **currentTaskSavedSp, void *newTaskSavedSp, void *currentTask, void *nextTask)
//
// Register Mapping (ARM AAPCS):
//   r0 = currentTaskSavedSp (Pointer to the variable storing the outgoing SP)
//   r1 = newTaskSavedSp     (The raw stack pointer value of the incoming task)
//   r2 = currentTask        (Pointer to outgoing TaskClass object - unused here)
//   r3 = nextTask           (Pointer to incoming TaskClass object - unused here)
//-----------------------------------------------------------------------------
.global TaskSwitch
.thumb_func
TaskSwitch:
    @ 1. Push callee-saved registers (r4-r11) AND the Link Register (lr) onto stack.
    push    {r4-r11, lr}

    @ 2. Store the updated Stack Pointer into *currentTaskSavedSp
    str     sp, [r0]

    @ 3. Switch the hardware stack pointer to the new task's SP passed in r1
    mov     sp, r1

    @ 4. Pop the new task's saved registers and pop saved LR directly into PC.
    @ This instantly branches execution into the resumed task.
    pop     {r4-r11, pc}

//-----------------------------------------------------------------------------
// StackType GetStackPointer()
// Returns the current hardware stack pointer value in r0.
//-----------------------------------------------------------------------------
.global GetStackPointer
.thumb_func
GetStackPointer:
    mov     r0, sp
    bx      lr
