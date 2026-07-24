# =============================================================================
# Tics Realtime Operating System - ARM Pure 64-bit (AArch64) Context Core
# =============================================================================

.global TaskSwitch
.global GetStackPointer
.type TaskSwitch, %function
.type GetStackPointer, %function

# -----------------------------------------------------------------------------
# 1. Context Switcher
# -----------------------------------------------------------------------------
TaskSwitch:
    # AArch64 PCS Calling Convention:
    # x0 = oldSp pointer (uintptr_t*)
    # x1 = newSp value (uintptr_t)

    # 1. Save Current Task Context
    # AArch64 pushes registers in 64-bit pairs while maintaining 16-byte alignment
    # Pre-index decrement by 16 bytes allocates space on the stack per pair
    stp x19, x20, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    stp x23, x24, [sp, #-16]!
    stp x25, x26, [sp, #-16]!
    stp x27, x28, [sp, #-16]!
    stp x29, x30, [sp, #-16]!   # x29 = Frame Pointer (fp), x30 = Link Register (lr)

    # 2. Save Old SP and Load New SP
    str sp, [x0]         # Store current CPU Stack Pointer into *oldSp
    mov sp, x1           # Load new task's SP value into the CPU Stack Pointer

    # 3. Restore New Task Context & Resume Task
    # Post-index increment reads values and frees 16 bytes per pair sequentially
    ldp x29, x30, [sp], #16     # Pops saved fp and lr (resumes tracking targets)
    ldp x27, x28, [sp], #16
    ldp x25, x26, [sp], #16
    ldp x23, x24, [sp], #16
    ldp x21, x22, [sp], #16
    ldp x19, x20, [sp], #16
    ret

# -----------------------------------------------------------------------------
# 2. Stack Pointer Utility (Compensates for 8-byte Return Address)
# -----------------------------------------------------------------------------
GetStackPointer:
    # 1. Copy the current SP to x0 (the designated return register)
    mov x0, sp
    
    # 2. Add 8 bytes to offset the 64-bit return address pushed by the call rule
    add x0, x0, #8
    ret

.end
