# =============================================================================
# Tics Realtime Operating System - RISC-V 32-bit (RV32) Context Core
# =============================================================================

.global TaskSwitch
.global GetStackPointer
.type TaskSwitch, %function
.type GetStackPointer, %function

# =============================================================================
# Tics Realtime Operating System - RISC-V 32-bit (RV32) Context Core
# =============================================================================

.global TaskSwitch
.global GetStackPointer
.type TaskSwitch, %function
.type GetStackPointer, %function

TaskSwitch:
    # -------------------------------------------------------------------------
    # 1. PUSH CONTEXT (Pure sequential pushes)
    # -------------------------------------------------------------------------
    addi sp, sp, -4
    sw   s11, 0(sp)
    addi sp, sp, -4
    sw   s10, 0(sp)
    addi sp, sp, -4
    sw   s9, 0(sp)
    addi sp, sp, -4
    sw   s8, 0(sp)
    addi sp, sp, -4
    sw   s7, 0(sp)
    addi sp, sp, -4
    sw   s6, 0(sp)
    addi sp, sp, -4
    sw   s5, 0(sp)
    addi sp, sp, -4
    sw   s4, 0(sp)
    addi sp, sp, -4
    sw   s3, 0(sp)
    addi sp, sp, -4
    sw   s2, 0(sp)
    addi sp, sp, -4
    sw   s1, 0(sp)
    addi sp, sp, -4
    sw   s0, 0(sp)
    addi sp, sp, -4
    sw   ra, 0(sp)      # Chronological last item pushed

    # -------------------------------------------------------------------------
    # 2. SAVE OLD SP AND LOAD NEW SP (With Dynamic Self-Switch Refresh)
    # -------------------------------------------------------------------------
    # Save the current, valid push pointer directly to RAM
    sw   sp, 0(a0)        

    # Compare CurrentTask (a2) and NextTask (a3)
    # If they are NOT equal, skip the reload and jump straight to standard_swap
    bne  a2, a3, standard_swap

    # If we get here, CurrentTask == NextTask (Self-Switch)
    # Update a1 with the fresh, deep sp value we just saved to 0(a0)
    lw   a1, 0(a0)        

standard_swap:
    # In either case, we update sp prior to popping
    mv   sp, a1           

    # -------------------------------------------------------------------------
    # 3. POP CONTEXT (Pure sequential pops)
    # -------------------------------------------------------------------------
    lw   ra, 0(sp)
    addi sp, sp, 4
    lw   s0, 0(sp)
    addi sp, sp, 4
    lw   s1, 0(sp)
    addi sp, sp, 4
    lw   s2, 0(sp)
    addi sp, sp, 4
    lw   s3, 0(sp)
    addi sp, sp, 4
    lw   s4, 0(sp)
    addi sp, sp, 4
    lw   s5, 0(sp)
    addi sp, sp, 4
    lw   s6, 0(sp)
    addi sp, sp, 4
    lw   s7, 0(sp)
    addi sp, sp, 4
    lw   s8, 0(sp)
    addi sp, sp, 4
    lw   s9, 0(sp)
    addi sp, sp, 4
    lw   s10, 0(sp)
    addi sp, sp, 4
    lw   s11, 0(sp)
    addi sp, sp, 4      
    ret

    # Critically reload ra from the stack frame slot 0(sp) one final time 
    # to clean up any register modifications caused by the print routine
    lw ra,  0(sp)       

# -----------------------------------------------------------------------------
# 2. Stack Pointer Utility (Returns the current hardware SP)
# -----------------------------------------------------------------------------
GetStackPointer:
    # Copy the true, unaltered hardware SP straight to the return register
    mv a0, sp
    ret

.end


