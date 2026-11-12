/* linker.ld */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack (End of 128KB SRAM) */
_estack = 0x20020000;    

/* Memory layout matching the STM32F446ZE */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 512K
  SRAM (rwx)      : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
  /* 1. Vector table and program code go into FLASH */
  .text :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Forces the vector table from startup file to stay at address 0x08000000 */
    *(.text)             /* All code from all .c files */
    *(.text*)            
    *(.rodata)           /* Constant data like strings or const variables */
    *(.rodata*)
    . = ALIGN(4);
    _etext = .;          /* Store the end address of flash text code */
  } > FLASH

  /* Used by the startup file to know where the data initialization values start in FLASH */
  _sidata = LOADADDR(.data);

  /* 2. Initialized data goes into SRAM but its initial values are stored in FLASH */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;          /* Start of RAM data area (used by startup file) */
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;          /* End of RAM data area (used by startup file) */
  } > SRAM AT > FLASH

  /* 3. Uninitialized data (zero filled) goes into SRAM */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;           /* Start of BSS area (used by startup file) */
    *(.bss)
    *(.bss*)
    . = ALIGN(4);
    _ebss = .;           /* End of BSS area (used by startup file) */
  } > SRAM
}
