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

    .data : AT (LOADADDR(.ARM.exidx) + SIZEOF(.ARM.exidx))
    {
        . = ALIGN(4);
        __data_start__ = .;
        _sdata = .;
        *(.vtable)
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
        __data_end__ = .;
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
        _ebss = .;
        __bss_end__ = .;
    } >RAM
}
