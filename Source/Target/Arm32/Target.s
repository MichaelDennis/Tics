//-----------------------------------------------------------------------------
// Copyright (c) 2026, Tics Realtime (Michael Dennis McDonnell)
// Target: ARM Cortex-M (Thumb-2) Cooperative Context Switch & Vectors
//-----------------------------------------------------------------------------

.syntax unified    @ Use modern unified ARM/Thumb assembly syntax
.cpu cortex-m3     @ Target the Cortex-M3 core instruction set
.thumb             @ Generate 16/32-bit Thumb-2 instructions

//-----------------------------------------------------------------------------
// External Linker Symbols
//-----------------------------------------------------------------------------
.extern _estack
.extern __bss_start__
.extern __bss_end__
.extern _sdata
.extern _edata
.extern _sidata
.extern __init_array_start
.extern __init_array_end
.extern main
.extern TimerTickIsr

//-----------------------------------------------------------------------------
// Vector Table Section (.isr_vector)
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
    .word Default_Handler      @ 0x38: PendSV Handler
    .word SysTick_Handler      @ 0x3C: SysTick Timer Handler

//-----------------------------------------------------------------------------
// Code Segment (.text)
//-----------------------------------------------------------------------------
.text

//-----------------------------------------------------------------------------
// Reset Handler: Explicit BSS/Data Initialization & Boot Handoff
//-----------------------------------------------------------------------------
.global Reset_Handler
.thumb_func
Reset_Handler:
    @ 1. Zero-fill the .bss section (initializes memory manager pool arrays)
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    movs    r2, #0
.bss_loop:
    cmp     r0, r1
    bge     .bss_done
    str     r2, [r0], #4
    b       .bss_loop
.bss_done:

    @ 2. Copy .data section from Flash to RAM
    ldr     r0, =_sdata
    ldr     r1, =_edata
    ldr     r2, =_sidata
.data_loop:
    cmp     r0, r1
    bge     .data_done
    ldr     r3, [r2], #4
    str     r3, [r0], #4
    b       .data_loop
.data_done:

    @ 3. Run global C++ constructors manually via .init_array table
    ldr     r0, =__init_array_start
    ldr     r1, =__init_array_end
.ctor_loop:
    cmp     r0, r1
    bge     .ctor_done
    ldr     r2, [r0], #4
    cmp     r2, #0
    beq     .ctor_next          @ Skip null function pointers
    push    {r0, r1}
    blx     r2
    pop     {r0, r1}
.ctor_next:
    b       .ctor_loop
.ctor_done:

    @ 4. Hand off to main application
    bl      main

    @ 5. Trap if main ever returns
.dead_loop:
    b       .dead_loop

//-----------------------------------------------------------------------------
// Default Handler: Catch-all trap for unexpected interrupts
//-----------------------------------------------------------------------------
.global Default_Handler
.thumb_func
Default_Handler:
    b       Default_Handler

//-----------------------------------------------------------------------------
// SysTick_Handler: 1ms system clock tick ISR
//-----------------------------------------------------------------------------
.global SysTick_Handler
.thumb_func
SysTick_Handler:
    push    {lr}
    bl      TimerTickIsr
    pop     {pc}

//-----------------------------------------------------------------------------
// TaskSwitch: Cooperative context switch routine
//-----------------------------------------------------------------------------
.global TaskSwitch
.thumb_func
TaskSwitch:
    push    {r4-r11, lr}
    str     sp, [r0]
    mov     sp, r1
    pop     {r4-r11, pc}

//-----------------------------------------------------------------------------
// GetStackPointer: Returns current SP
//-----------------------------------------------------------------------------
.global GetStackPointer
.thumb_func
GetStackPointer:
    mov     r0, sp
    bx      lr
