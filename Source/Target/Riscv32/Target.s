# =============================================================================
# Tics Realtime Operating System - RISC-V 32-bit (RV32) Context Core
# =============================================================================

.global TaskSwitch
.global GetStackPointer
.type TaskSwitch, %function
.type GetStackPointer, %function

# -----------------------------------------------------------------------------
# 1. Context Switcher
# -----------------------------------------------------------------------------
TaskSwitch:
    # RISC-V ABI Calling Convention:
    # a0 = oldSp pointer (uintptr_t*)
    # a1 = newSp value (uintptr_t)

    # 1. Save Current Task Context
    addi sp, sp, -56    # Allocate space for 14 registers (14 * 4 bytes)
    sw ra,  0(sp)
    sw s0,  4(sp)
    sw s1,  8(sp)
    sw s2,  12(sp)
    sw s3,  16(sp)
    sw s4,  20(sp)
    sw s5,  24(sp)
    sw s6,  28(sp)
    sw s7,  32(sp)
    sw s8,  36(sp)
    sw s9,  40(sp)
    sw s10, 44(sp)
    sw s11, 48(sp)

    # 2. Save Old SP and Load New SP
    sw sp, 0(a0)        # Store current Stack Pointer into *oldSp
    mv sp, a1           # Load new task's SP into our Stack Pointer register

    # 3. Restore New Task Context & Resume Task
    lw ra,  0(sp)
    lw s0,  4(sp)
    lw s1,  8(sp)
    lw s2,  12(sp)
    lw s3,  16(sp)
    lw s4,  20(sp)
    lw s5,  24(sp)
    lw s6,  28(sp)
    lw s7,  32(sp)
    lw s8,  36(sp)
    lw s9,  40(sp)
    lw s10, 44(sp)
    lw s11, 48(sp)
    addi sp, sp, 56     # Free the stack frame
    ret

# -----------------------------------------------------------------------------
# 2. Stack Pointer Utility (Compensates for 4-byte Return Address)
# -----------------------------------------------------------------------------
GetStackPointer:
    # 1. Copy the current SP to a0 (the designated return register)
    mv a0, sp
    
    # 2. Add 4 bytes to offset the return address pushed to the stack by the call
    addi a0, a0, 4
    ret

.end
