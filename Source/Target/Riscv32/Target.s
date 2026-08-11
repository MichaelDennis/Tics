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
    # 2.1 If the CurrentTask and NextTask are the same, then the SavedSp 
    # of NextTask, that was passed to this function as a1 will be incorrect 
    # because this function moves the sp down to save the CurrentTask's 
    # registers. 
    #
    # So, in the code below, if the CurentTask's address (a2) is not the 
    # same as the NextTask's address a3, then no adjustment to NextTask's 
    # SavedSp is required, and the adjustment of NexTask's SavedSp is skipped. 
    #
    # However, if CurrentTask and NextTask are the same, we must update the 
    # value of NextTask's SaveSp, which is stored in register a1, so that the 
    # restore of NextTask's registers is done correctly.
    #
    # Notes: (1) When CurrentTask and NextTask are the same, a reference to 
    # CurrentTask or NextTask is a reference to the same task. (2) The main
    # way in which CurrentTask and NextTask are the same is when there is
    # only one user task in the system, and its task loop simply contains
    # a call to Yield(). In that case, the task being swapped out and the
    # task being swapped in are the same.
    # -------------------------------------------------------------------------
   # If currentTask (a2) and nextTask (a3) pointers are different, then skip the adjustment.
    bne  a2, a3, skipNextTaskSavedSpUpdate

    # Adjust NextTask->SavedSp for the case where we are switching back to the same task.
    lw   a1, 0(a0)        

skipNextTaskSavedSpUpdate:
    # In either case, we update NextTask's SavedSp prior to restoring its registers.
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
