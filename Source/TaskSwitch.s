    # -------------------------------------------------------------------------
    # Call this from C/C++ as shown below.
    #
    # extern "C" void TaskSwitch(void** currentTaskSavedSp, void* newTaskSavedSp);
    # -------------------------------------------------------------------------

.global TaskSwitch
.type TaskSwitch, @function

TaskSwitch:
    # -------------------------------------------------------------------------
    # 1. SAVE CURRENT TASK CONTEXT
    # -------------------------------------------------------------------------
    # Push the 4 callee-saved registers onto the current stack.
    # Each push subtracts 4 bytes from ESP. Total = 16 bytes.
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    # -------------------------------------------------------------------------
    # 2. SAVE OLD SP AND LOAD NEW SP
    # -------------------------------------------------------------------------
    # Because we just pushed 16 bytes, our arguments on the stack have shifted:
    #   [ESP + 0]  = EDI
    #   [ESP + 4]  = ESI
    #   [ESP + 8]  = EBX
    #   [ESP + 12] = EBP
    #   [ESP + 16] = Return Address (Caller of TaskSwitch)
    #   [ESP + 20] = currentTaskSavedSp (void**)
    #   [ESP + 24] = newTaskSavedSp     (void*)

    movl 20(%esp), %eax    # EAX = address of currentTaskSavedSp variable
    movl %esp, (%eax)      # Save current ESP into *currentTaskSavedSp

    movl 24(%esp), %esp    # Load the new task's saved SP into the ESP register
                           # We are now officially running on the new stack!

    # -------------------------------------------------------------------------
    # 3. RESTORE NEW TASK CONTEXT
    # -------------------------------------------------------------------------
    # Pop the 4 callee-saved registers off the new stack.
    # Each pop adds 4 bytes to ESP. Total = 16 bytes.
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp

    # -------------------------------------------------------------------------
    # 4. RESUME THE NEW TASK
    # -------------------------------------------------------------------------
    # ESP is now pointing exactly at the return address.
    # This pops the 4-byte address and jumps to it.
    ret

.global PrimeTaskStack
.type PrimeTaskStack, @function

PrimeTaskStack:
    # -------------------------------------------------------------------------
    # 1. FETCH ARGUMENTS FROM THE C++ CALLER
    # -------------------------------------------------------------------------
    # Upon entry, the arguments sit on the stack relative to the current ESP:
    #   [esp + 4]  = stackBuffer (void*)  -> Pointer to start of allocated memory
    #   [esp + 8]  = bufferSize  (size_t) -> Size of the allocated buffer
    #   [esp + 12] = taskFunction(void*)  -> Address of the C++ static wrapper
    
    movl 4(%esp), %eax    # %eax = stackBuffer base address
    movl 8(%esp), %ecx    # %ecx = bufferSize
    movl 12(%esp), %edx   # %edx = taskFunction pointer

    # -------------------------------------------------------------------------
    # 2. CALCULATE AND ALIGN THE STACK TOP
    # -------------------------------------------------------------------------
    addl %ecx, %eax       # %eax = stackBuffer + bufferSize (Top of the stack)
    andl $0xFFFFFFF0, %eax # Force 16-byte alignment (Clears lower 4 bits to 0x0)

    # -------------------------------------------------------------------------
    # 3. CONSTRUCT THE PRIMED STACK FRAME
    # -------------------------------------------------------------------------
    # To satisfy the 16-byte alignment rule for when the task launches:
    # We apply 12 bytes of padding (ends in 0x4), then 20 bytes for the frame.
    # Total space subtracted = 32 bytes (a perfect multiple of 16, so it ends in 0x0).
    subl $32, %eax        # Carve out the space

    # Now we populate the memory offsets relative to our new stack top (%eax):
    #   [%eax + 28] = Dead space padding / Return trap placeholder
    #   [%eax + 24] = Dead space padding / Return trap placeholder
    #   [%eax + 20] = Dead space padding / Return trap placeholder
    #   [%eax + 16] = taskFunction address (What your assembly 'ret' pops)
    #   [%eax + 12] = Initial EBP value (0)
    #   [%eax + 8]  = Initial EBX value (0)
    #   [%eax + 4]  = Initial ESI value (0)
    #   [%eax + 0]  = Initial EDI value (0)

    movl %edx, 16(%eax)   # Write taskFunction to the 'ret' execution slot
    
    # Initialize the 4 callee-saved register slots to 0
    movl $0, 12(%eax)     # Initial EBP
    movl $0, 8(%eax)      # Initial EBX
    movl $0, 4(%eax)      # Initial ESI
    movl $0, 0(%eax)      # Initial EDI

    # -------------------------------------------------------------------------
    # 4. RETURN THE PRIMED STACK POINTER
    # -------------------------------------------------------------------------
    # In the C/C++ calling convention, the return value of a function 
    # must be placed in the %eax register. 
    # %eax already holds the exact memory address of the bottom of our new frame.
    ret

# Clean footer for the Linux linker
.section .note.GNU-stack,"",@progbits

/*
How to Declare It in Your C++ CodeTo use this assembly version, you declare it as an external C function in your header file, exactly like you did for TaskSwitch:cppextern "C" {
    // Takes the memory array, primes it in assembly, and returns the new top pointer
    void* PrimeTaskStack(void* stackBuffer, size_t bufferSize, void (*taskFunction)());
}

// Example usage when spawning a new task:
void* myNewStackMemory = malloc(4096); 
void* savedEspForNewTask = PrimeTaskStack(myNewStackMemory, 4096, &MyTaskClass::StaticTaskRunner);

// You can now feed 'savedEspForNewTask' directly into your TaskSwitch function!


*/
