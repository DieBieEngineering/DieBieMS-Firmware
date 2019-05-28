# DieBieMS-Firmware

This is the firmware for this hardware : https://github.com/DieBieEngineering/DieBieMS <-look here for a project description..

When combined with this bootloader : https://github.com/DieBieEngineering/DieBieMS-Bootloader

You can upgrade the firmware trough USB with the VESC-Tool from this project : https://github.com/vedderb/vesc_tool 
The VESC-tool in binary form can be sourced from the VESC project page : http://www.vesc-project.com

When attempting to install this firmware and bootloader you need to know the flash mapping. Make sure to use a microcontroller with at leas 256kB of flash (eg the STM32F303CCT6 as described by the bom). When you use a lower size you cannot use the bootloader (flash is to small), when you use higher you don't utilize all the potential flash area.

When flashing the application the start address should be: <b>0x08000000</b>
When flashing the bootloader the start address should be: <b>0x08032000</b>

The flash is formatted as follows (summary):

#define ADDR_FLASH_PAGE_0   ((uint32_t)0x08000000) /* Base @ of Page 0, 2 Kbytes */  // Startup Code - Main application

#define ADDR_FLASH_PAGE_1   ((uint32_t)0x08000800) /* Base @ of Page 1, 2 Kbytes */  // Page0 - EEPROM emulation

#define ADDR_FLASH_PAGE_2   ((uint32_t)0x08001000) /* Base @ of Page 2, 2 Kbytes */  // Page1 - EEPROM emulation

#define ADDR_FLASH_PAGE_3   ((uint32_t)0x08001800) /* Base @ of Page 3, 2 Kbytes */  // Remainder of the main application firmware stars
from here.

#define ADDR_FLASH_PAGE_50  ((uint32_t)0x08019000) /* Base @ of Page 50, 2 Kbytes */  // New app firmware base addres

#define ADDR_FLASH_PAGE_100 ((uint32_t)0x08032000) /* Base @ of Page 100, 2 Kbytes */  // Bootloader base

See "modFlash.h" and "modFlash.c" for more info.
