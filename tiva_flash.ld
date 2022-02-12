/*
 * Linker script.
 *
 * Puts code in flash and the rest in RAM.
 */

MEMORY {
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00040000
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000
}

SECTIONS {
    /* code and constants in flash */
    .text : {
        _text = .;
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH

    /* initialized data in sram */
    .data : AT(ADDR(.text) + SIZEOF(.text)) {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM

    /* uninitialized data in sram */
    .bss : {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM
}
