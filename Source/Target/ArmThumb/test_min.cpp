// Minimal ARM Cortex-M3 bare-metal baseline with main()
extern "C" void Reset_Handler();
extern "C" void Default_Handler()
{
    while (1)
        ;
}

// Linker script symbols for BSS clearing
extern "C" unsigned long __bss_start__;
extern "C" unsigned long __bss_end__;

// Vector Table
__attribute__((section(".isr_vector"), used)) void (*const exception_vectors[])(void) = {
    (void (*)(void))0x20040000, // Initial Stack Pointer
    Reset_Handler,              // Reset Handler
    Default_Handler,            // NMI
    Default_Handler,            // HardFault
};

int main(void)
{
    volatile int counter = 0;
    while (1)
    {
        counter++;
    }
}

extern "C" void Reset_Handler(void)
{
    // Zero out BSS section
    unsigned long *bss_ptr = &__bss_start__;
    unsigned long *bss_end = &__bss_end__;
    while (bss_ptr < bss_end)
    {
        *bss_ptr++ = 0;
    }

    // Jump to main
    main();

    while (1)
        ;
}
