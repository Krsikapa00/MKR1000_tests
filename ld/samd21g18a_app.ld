/*
 * Linker script for ATSAMD21G18A applications loaded by the Arduino SAM-BA
 * bootloader. The bootloader occupies the first 8 KB of flash, so the
 * application (and its vector table) starts at 0x2000.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00002000, LENGTH = 256K - 8K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

ENTRY(Reset_Handler)

__stack_top = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.vectors))
        *(.text .text.*)
        *(.rodata .rodata.*)

        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
        . = ALIGN(4);
    } > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH

    __data_load = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT > FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    /* Fail the link if globals leave less than 4 KB for the stack. */
    ASSERT(__bss_end <= __stack_top - 4K, "Not enough RAM left for the stack")
}
