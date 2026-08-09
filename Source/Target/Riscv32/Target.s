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
    
    # Save the 13 context registers relative to the new bottom baseline
    sw   ra,  0(sp)     # ra sits cleanly at the lowest address slot (Offset 0)
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
    # 2. SAVE OLD SP AND LOAD NEW SP (With Your 4-Arg Verification Fix)
    # -------------------------------------------------------------------------
    # Save the current, valid baseline frame pointer directly to RAM
    sw   sp, 0(a0)        

    # Compare currentTask (a2) and nextTask (a3) pointers
    bne  a2, a3, standard_swap

    # Self-Switch: Force-refresh a1 out of RAM to bypass the stale C++ parameter
    lw   a1, 0(a0)        

standard_swap:
    # In either case, we update sp prior to popping
    mv   sp, a1           

    # -------------------------------------------------------------------------
    # 3. RESTORE CONTEXT (Indexed block restoration)
    # -------------------------------------------------------------------------
    lw   ra,  0(sp)     # Reads from offset 0
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
# 2. Stack Pointer Utility 
# -----------------------------------------------------------------------------
GetStackPointer:
    mv a0, sp
    ret

.end
