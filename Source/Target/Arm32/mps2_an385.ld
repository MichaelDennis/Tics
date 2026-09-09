/* Linker script for ARM Cortex-M3 (QEMU mps2-an385) */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(Reset_Handler)

SECTIONS
{
    .isr_vector ORIGIN(FLASH) :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    /* PRIORITIZE CONSTRUCTORS: Placed above .text to prevent wildcard capture */
    .init_array :
    {
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
        __init_array_end = .;
    } >FLASH

    .fini_array :
    {
        . = ALIGN(4);
        __fini_array_start = .;
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        KEEP(*(SORT(.fini_array)))
        __fini_array_end = .;
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.init)
        *(.init*)
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.fini)
        *(.fini*)
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .ARM.exidx :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } >FLASH

    /* Define load address of .data in Flash for copying */
    _sidata = LOADADDR(.data);

    .data : AT (LOADADDR(.ARM.exidx) + SIZEOF(.ARM.exidx))
    {
        . = ALIGN(4);
        __data_start__ = .;
        _sdata = .;
        *(.vtable)
        *(.data)
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
        _edata = .;
    } >RAM

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
        _ebss = .;
        _end = .;
    } >RAM
}
