# =============================================================================
# Tics Realtime Operating System - RISC-V 32-bit (RV32) Context Core
# =============================================================================

.global TaskSwitch
.global GetStackPointer
.type TaskSwitch, %function
.type GetStackPointer, %function

TaskSwitch:
    # -------------------------------------------------------------------------
    # 1. SAVE CONTEXT (Indexed block allocation - 16-byte aligned)
    # -------------------------------------------------------------------------
    addi sp, sp, -56    # Allocate 56-byte frame (52 bytes data + 4 bytes padding)
    
    # Save the currentTask's registers on the stack.
    sw   ra,  0(sp)     
    sw   s0,  4(sp)
    sw   s1,  8(sp)
    sw   s2,  12(sp)
    sw   s3,  16(sp)
    sw   s4,  20(sp)
    sw   s5,  24(sp)
    sw   s6,  28(sp)
    sw   s7,  32(sp)
    sw   s8,  36(sp)
    sw   s9,  40(sp)
    sw   s10, 44(sp)
    sw   s11, 48(sp)    # Chronological last register (Leaves 52-55 as empty padding)

    # -------------------------------------------------------------------------
    # 2. Save sp to CurrentTask's SavedSp so that the registers can be
    # restored properly to CurrentTask when it is resumed. Note that a0
    # contains the address of CurrentTask->Stack.SavedSp.
    # -------------------------------------------------------------------------
    sw   sp, 0(a0)        

    # -------------------------------------------------------------------------
    # 2.1 Update NextTask's SavedSp prior to restoring its registers.
    # -------------------------------------------------------------------------
    mv   sp, a1           

    # -------------------------------------------------------------------------
    # 3. Restore NextTask's registers.
    # -------------------------------------------------------------------------
    lw   ra,  0(sp)
    lw   s0,  4(sp)
    lw   s1,  8(sp)
    lw   s2,  12(sp)
    lw   s3,  16(sp)
    lw   s4,  20(sp)
    lw   s5,  24(sp)
    lw   s6,  28(sp)
    lw   s7,  32(sp)
    lw   s8,  36(sp)
    lw   s9,  40(sp)
    lw   s10, 44(sp)
    lw   s11, 48(sp)
    
    # Free the 56-byte aligned block entirely, restoring parent stack alignment
    addi sp, sp, 56     
    ret

# -----------------------------------------------------------------------------
# Function to return the hardware stack pointer (sp).
# -----------------------------------------------------------------------------
GetStackPointer:
    mv a0, sp
    ret

.end
