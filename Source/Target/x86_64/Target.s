# =============================================================================
# Tics Realtime Operating System - x86_64 64-bit Linux Hosted Context Switch
# =============================================================================

.global TaskSwitch
.type TaskSwitch, @function

TaskSwitch:
    # 1. Save Current Task Context (System V AMD64 ABI Callee-saved registers)
    pushq %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # System V ABI passes parameters directly in registers:
    # rdi = oldSp pointer (uintptr_t*)
    # rsi = newSp value (uintptr_t)

    # 2. Save Old SP and Load New SP
    movq %rsp, (%rdi)     # Store current Stack Pointer into *oldSp
    movq %rsi, %rsp       # Load new task's SP into the CPU Stack Pointer

    # 3. Restore New Task Context & Resume Task
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret

.global GetStackPointer
.type GetStackPointer, %function

GetStackPointer:
    # 1. Copy current SP to return register
    movq %rsp, %rax
    
    # 2. Add 8 bytes to offset the 64-bit return address pushed by the call instruction
    addq $8, %rax
    ret

.end
