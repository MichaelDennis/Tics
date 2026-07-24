/* ============================================================================
   Tics Realtime Operating System - ARM Cortex-M (Thumb-2) Context Core
   ============================================================================ */

.syntax unified
.global TaskSwitch
.global GetStackPointer
.type TaskSwitch, %function
.type GetStackPointer, %function
.thumb                  @ Force the assembler engine into Thumb mode
.thumb_func             @ Required to mark entry points for Thumb execution

# -----------------------------------------------------------------------------
# 1. Context Switcher
# -----------------------------------------------------------------------------
TaskSwitch:
    @ AAPCS Calling Convention:
    @ r0 = oldSp pointer (uintptr_t*)
    @ r1 = newSp value (uintptr_t)

    @ 1. Save Current Task Context (Callee-saved r4-r11 + active Link Register)
    push {r4-r11, lr}

    @ 2. Save Old SP and Load New SP
    str sp, [r0]        @ Store current CPU stack pointer into *oldSp
    mov sp, r1          @ Load new task's SP value into the CPU Stack Pointer

    @ 3. Restore New Task Context & Resume Task
    pop {r4-r11, pc}    @ Pops saved lr straight into pc to resume execution

# -----------------------------------------------------------------------------
# 2. Stack Pointer Utility (Compensates for 4-byte Return Address)
# -----------------------------------------------------------------------------
GetStackPointer:
    @ 1. Copy the current SP to r0 (the designated return register)
    mov r0, sp
    
    @ 2. Add 4 bytes to offset the return address pushed to the stack by the call
    add r0, r0, #4
    bx lr

.end
