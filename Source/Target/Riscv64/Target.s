# =============================================================================
# Tics Realtime Operating System - RISC-V 64-bit (RV64) Context Core
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
    addi sp, sp, -112   # Allocate space for 14 registers (14 * 8 bytes)
    sd ra,  0(sp)
    sd s0,  8(sp)
    sd s1,  16(sp)
    sd s2,  24(sp)
    sd s3,  32(sp)
    sd s4,  40(sp)
    sd s5,  48(sp)
    sd s6,  56(sp)
    sd s7,  64(sp)
    sd s8,  72(sp)
    sd s9,  80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)

    # 2. Save Old SP and Load New SP
    sd sp, 0(a0)        # Store current Stack Pointer into *oldSp
    mv sp, a1           # Load new task's SP into our Stack Pointer register

    # 3. Restore New Task Context & Resume Task
    ld ra,  0(sp)
    ld s0,  8(sp)
    ld s1,  16(sp)
    ld s2,  24(sp)
    ld s3,  32(sp)
    ld s4,  40(sp)
    ld s5,  48(sp)
    ld s6,  56(sp)
    ld s7,  64(sp)
    ld s8,  72(sp)
    ld s9,  80(sp)
    ld s10, 88(sp)
    ld s11, 96(sp)
    addi sp, sp, 112    # Free the stack frame
    ret

# -----------------------------------------------------------------------------
# 2. Stack Pointer Utility (Compensates for 8-byte Return Address)
# -----------------------------------------------------------------------------
GetStackPointer:
    # 1. Copy the current SP to a0 (the designated return register)
    mv a0, sp
    
    # 2. Add 8 bytes to offset the 64-bit return address pushed by the call
    addi a0, a0, 8
    ret

.end
