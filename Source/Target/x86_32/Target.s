#-----------------------------------------------------------------------------
# Copyright (c) 2026, Tics Realtime (Michael Dennis McDonnell)
# Target: x86 32-bit (IA32) Hosted Cooperative Context Switch
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Target.s
#
# This file contains Tics x86_32 platform functions that must be written 
# in assembly language.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# External Linker Symbols
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Globals
#-----------------------------------------------------------------------------

# Export the task switching function.
.global TaskSwitch

# Export the GetStackPointer function which returns the current value of SP.
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
#   Saves the current Tics task's registers and restores the incoming 
#   task's registers.
#
# Arguments:
#   20(%esp) - currentTaskSavedSp: Pointer to save location for current task SP.
#   24(%esp) - newTaskSavedSp: SP value of the incoming (new) task.
#-----------------------------------------------------------------------------
TaskSwitch:
    # 1. Save Current Task Context (Callee-saved registers)
    pushl %ebp            # Save the base pointer register state.
    pushl %ebx            # Save the base register state.
    pushl %esi            # Save the source index register state.
    pushl %edi            # Save the destination index register state.

    # Grab parameters off the stack frame matching cdecl conventions.
    movl 20(%esp), %eax   # Load the oldSp pointer into register eax.
    movl 24(%esp), %edx   # Load the newSp value into register edx.

    # 2. Save Old SP and Load New SP
    movl %esp, (%eax)     # Store current Stack Pointer into *oldSp.
    movl %edx, %esp       # Load new task's SP into the CPU Stack Pointer.

    # 3. Restore New Task Context & Resume Task
    popl %edi             # Restore the destination index register.
    popl %esi             # Restore the source index register.
    popl %ebx             # Restore the base register.
    popl %ebp             # Restore the base pointer register.
    ret                   # Return to the newly loaded task execution path.

#-----------------------------------------------------------------------------
# Get Stack Pointer: Returns current SP.
#
# Description:
#   Return the raw, unadjusted stack pointer (SP) value of the current task. 
#   The target-specific C++ encapsulation wrapper layer will handle the 
#   calling-convention return address compensation math dynamically.
#
# Return Value:
#   %eax - The raw unadjusted SP value of the current task.
#-----------------------------------------------------------------------------
GetStackPointer:
    # Capture the execution stack address state.
    movl %esp, %eax       # Move the current task's SP value into register eax.
    ret                   # Return back to the calling application environment.

.end
