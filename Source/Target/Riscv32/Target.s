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

#-----------------------------------------------------------------------------
# File Target.s
#
# This file contains core assembler directives for configuring the 
# RISC-V 32-bit target platform characteristics.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# External Linker Symbols
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Globals
#-----------------------------------------------------------------------------

# Export the Task Switch cooperative scheduling symbol.
.global TaskSwitch

# Export the Get Stack Pointer diagnostics symbol.
.global GetStackPointer

# Declare TaskSwitch as a function type symbol identifier.
.type TaskSwitch, %function

# Declare GetStackPointer as a function type symbol identifier.
.type GetStackPointer, %function

#-----------------------------------------------------------------------------
# Statics
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Functions
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Task Switch: Cooperative context switch routine.
#
# Description:
#   Switches CPU context execution frames between two active kernel threads.
#   Allocates an aligned stack frame, stores the callee-saved registers, 
#   swaps active stack pointers, and restores the incoming context.
#
# Arguments:
#   a0 - currentTaskSavedSp: Pointer to save location for current task SP.
#   a1 - newTaskSavedSp: Raw stack address of the incoming task.
#-----------------------------------------------------------------------------
TaskSwitch:
    # 1. SAVE CONTEXT (Indexed block allocation - 16-byte aligned)
    # Allocate 56-byte frame (52 bytes data + 4 bytes padding).
    addi sp, sp, -56
    # Save the currentTask's registers on the stack.
    sw ra, 0(sp)
    sw s0, 4(sp)
    sw s1, 8(sp)
    sw s2, 12(sp)
    sw s3, 16(sp)
    sw s4, 20(sp)
    sw s5, 24(sp)
    sw s6, 28(sp)
    sw s7, 32(sp)
    sw s8, 36(sp)
    sw s9, 40(sp)
    sw s10, 44(sp)
    # Chronological last register (Leaves 52-55 as empty padding).
    sw s11, 48(sp)

    # 2. Save sp to CurrentTask's SavedSp so that the registers can be
    # restored properly to CurrentTask when it is resumed. Note that a0
    # contains the address of CurrentTask->Stack.SavedSp.
    sw sp, 0(a0)

    # 2.1 Update NextTask's SavedSp prior to restoring its registers.
    mv sp, a1

    # 3. Restore NextTask's registers.
    lw ra, 0(sp)
    lw s0, 4(sp)
    lw s1, 8(sp)
    lw s2, 12(sp)
    lw s3, 16(sp)
    lw s4, 20(sp)
    lw s5, 24(sp)
    lw s6, 28(sp)
    lw s7, 32(sp)
    lw s8, 36(sp)
    lw s9, 40(sp)
    lw s10, 44(sp)
    lw s11, 48(sp)

    # Free the 56-byte aligned block entirely, restoring parent stack alignment.
    addi sp, sp, 56
    ret

#-----------------------------------------------------------------------------
# Get Stack Pointer: Returns current SP.
#
# Description:
#   Query routine pulling the active address layer of the hardware stack 
#   pointer register for diagnostic validation checks.
#
# Return Value:
#   a0 - The current value of the CPU stack pointer register.
#-----------------------------------------------------------------------------
GetStackPointer:
    # Move the active Stack Pointer address into register a0 for the return value.
    mv a0, sp
    ret

.end
