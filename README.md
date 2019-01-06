# Other parts of this project
This is the firmware repository containing all firmware source files. There are three more repositories for this project:<br>
[DieBieMS Hardware](https://github.com/DieBieEngineering/DieBieMS) The hardware sourcefiles.<br>
[DieBieMS Bootloader](https://github.com/DieBieEngineering/DieBieMS-Bootloader) can be flashed with the BMS Tool in the firmware tab.<br>
[DieBieMS Configuration tool](https://github.com/DieBieEngineering/DieBieMS-Tool) the tool to configure the BMS and upload the bootloader / update the main firmware.

When attempting to install this firmware and bootloader you need to know the flash mapping. Make sure to use a microcontroller with at least 256kB of flash (eg the STM32F303CCT6 as described by the bom). When you use a lower size you cannot use the bootloader (flash is to small), when you use higher you don't utilize all the potential flash area.

When flashing the application the start address should be: <b>0x08000000</b>
When flashing the bootloader the start address should be: <b>0x08032000</b>

The flash is formatted as follows (summary):

((uint32_t)0x08000000) /* Base @ of Page 0, 2 Kbytes */  // Startup Code - Main application<br>
((uint32_t)0x08000800) /* Base @ of Page 1, 2 Kbytes */  // Page0 - EEPROM emulation<br>
((uint32_t)0x08001000) /* Base @ of Page 2, 2 Kbytes */  // Page1 - EEPROM emulation<br>
((uint32_t)0x08001800) /* Base @ of Page 3, 2 Kbytes */  // Remainder of the main application firmware stars from here.<br>
((uint32_t)0x08019000) /* Base @ of Page 50, 2 Kbytes */  // New app firmware base addres<br>
((uint32_t)0x08032000) /* Base @ of Page 100, 2 Kbytes */  // Bootloader base<br>

See "modFlash.h" and "modFlash.c" for more info.
