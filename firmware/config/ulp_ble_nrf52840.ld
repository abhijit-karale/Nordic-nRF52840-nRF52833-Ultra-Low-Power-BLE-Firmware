/* Linker script for Nordic nRF52840 with SoftDevice S140 and .noinit.retention section */

SEARCH_DIR(.)
GROUP(libgcc.a libc.a libm.a libnosys.a)

MEMORY
{
  FLASH (rx)          : ORIGIN = 0x26000, LENGTH = 0xB4000   /* 720 kB app flash after S140 */
  RAM (rwx)           : ORIGIN = 0x20002200, LENGTH = 0x3DC00 /* ~247 kB app RAM */
  RETAINED_RAM (rw)   : ORIGIN = 0x2003FE00, LENGTH = 0x200   /* 512 bytes retained across System OFF */
}

SECTIONS
{
  . = ALIGN(4);
  .noinit.retention (NOLOAD) :
  {
    PROVIDE(__start_noinit_retention = .);
    *(.noinit.retention*)
    PROVIDE(__stop_noinit_retention = .);
  } > RETAINED_RAM

  .noinit.fault_record (NOLOAD) :
  {
    PROVIDE(__start_noinit_fault = .);
    *(.noinit.fault_record*)
    PROVIDE(__stop_noinit_fault = .);
  } > RETAINED_RAM
}

SECTIONS
{
  .text :
  {
    KEEP(*(.vectors))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .data : AT (__etext)
  {
    __data_start__ = .;
    *(.data*)
    . = ALIGN(4);
    __data_end__ = .;
  } > RAM

  __etext = LOADADDR(.data);

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM

  .noinit (NOLOAD) :
  {
    *(.noinit*)
  } > RAM
}

ENTRY(Reset_Handler)
