# =============================================================================
# Tics Realtime Operating System - x86 32-bit (IA32) Hosted Context Switch
# =============================================================================

.global TaskSwitch
.type TaskSwitch, @function

TaskSwitch:
    # 1. Save Current Task Context (Callee-saved registers)
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    # Grab parameters off the stack frame (cdecl calling convention)
    movl 20(%esp), %eax   # eax = oldSp pointer (uintptr_t*)
    movl 24(%esp), %edx   # edx = newSp value (uintptr_t)

    # 2. Save Old SP and Load New SP
    movl %esp, (%eax)     # Store current Stack Pointer into *oldSp
    movl %edx, %esp       # Load new task's SP into the CPU Stack Pointer

    # 3. Restore New Task Context & Resume Task
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.global GetStackPointer
.type GetStackPointer, %function

GetStackPointer:
    movl %esp, %eax
    addl $4, %eax
    ret
.end
