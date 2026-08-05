# =============================================================================
# Tics Realtime Operating System - RISC-V 32-bit (RV32) Context Core
# =============================================================================

.global TaskSwitch
.global GetStackPointer
.type TaskSwitch, %function
.type GetStackPointer, %function

TaskSwitch:
    # 1. Save Current Task Context (56 is perfectly divisible by 16)
    addi sp, sp, -56    
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
    sw s11, 48(sp)      # Leaves bytes 52-55 as empty alignment padding

    # 2. Save Old SP and Load New SP
    sw sp, 0(a0)        
    mv sp, a1           

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
    addi sp, sp, 56     # Free the aligned frame
    ret

# -----------------------------------------------------------------------------
# 2. Stack Pointer Utility (Returns the current hardware SP)
# -----------------------------------------------------------------------------
GetStackPointer:
    # Copy the true, unaltered hardware SP straight to the return register
    mv a0, sp
    ret


.equ CLINT_MTIME, 0x02004ff8

.global read_mtime_counter
read_mtime_counter:
    li   t0, CLINT_MTIME
    lw   a1, 4(t0)            # High 32 bits of mtime in a1
    lw   a0, 0(t0)            # Low 32 bits of mtime in a0
                              # Units: 100 ns ticks (10 MHz clock frequency on QEMU virt)
                              # 10,000 ticks = 1 ms
    ret    

.end
