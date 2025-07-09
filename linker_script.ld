/* Linker script for Stellaris LM3S6965EVB (ARM Cortex-M4) */
/* Memory layout: 256KB Flash, 64KB SRAM */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Stack size configuration */
_stack_size = 0x2000; /* 8KB stack - our allocated VM memory */

/* Entry point */
ENTRY(Reset_Handler)

SECTIONS
{
    /* Vector table and code in Flash */
    .text :
    {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* Data section initialization */
    .data :
    {
        _data_start = .;
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } > SRAM AT > FLASH

    _data_load = LOADADDR(.data);

    /* BSS section */
    .bss :
    {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > SRAM

    /* Heap for dynamic allocation (for C++ new/delete) */
    _heap_start = _bss_end;
    _heap_end = _stack_end - 0x1000; /* Leave 4KB gap before stack */
    
    /* Stack at end of SRAM */
    _stack_start = ORIGIN(SRAM) + LENGTH(SRAM);
    _stack_end = _stack_start - _stack_size;

    /* VM memory region (8KB as planned) */
    _vm_memory_start = 0x20000000;
    _vm_memory_size = 0x2000; /* 8KB */
    _vm_memory_end = _vm_memory_start + _vm_memory_size;
}