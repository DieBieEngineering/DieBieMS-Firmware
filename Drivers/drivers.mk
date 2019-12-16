VPATH += Drivers/HWDrivers/Src/ \
		 Drivers/SWDrivers/Src/ \
		 Drivers/STM32F3xx_HAL_Driver/Src/ \

HWDRIVER_SRC = 	Drivers/HWDrivers/Src/driverHWADC.c \
				Drivers/HWDrivers/Src/driverHWEEPROM.c \
				Drivers/HWDrivers/Src/driverHWI2C1.c \
				Drivers/HWDrivers/Src/driverHWI2C2.c \
				Drivers/HWDrivers/Src/driverHWPowerState.c \
				Drivers/HWDrivers/Src/driverHWSPI1.c \
				Drivers/HWDrivers/Src/driverHWStatus.c \
				Drivers/HWDrivers/Src/driverHWSwitches.c \
				Drivers/HWDrivers/Src/driverHWUART2.c \
				Drivers/HWDrivers/Src/driverHWWS2812.c \
#$(wildcard Drivers/HWDrivers/Src/driver*.c)

HWDRIVER_INC =	Drivers/HWDrivers/Inc

SWDRIVER_SRC =	Drivers/SWDrivers/Src/driverSWAD5245.c \
				Drivers/SWDrivers/Src/driverSWADC128D818.c \
				Drivers/SWDrivers/Src/driverSWADS1015.c \
				Drivers/SWDrivers/Src/driverSWCC1101.c \
				Drivers/SWDrivers/Src/driverSWDCDC.c \
				Drivers/SWDrivers/Src/driverSWEMC2305.c \
				Drivers/SWDrivers/Src/driverSWISL28022.c \
				Drivers/SWDrivers/Src/driverSWLTC6803.c \
				Drivers/SWDrivers/Src/driverSWLTC6804.c \
				Drivers/SWDrivers/Src/driverSWMCP3221.c \
				Drivers/SWDrivers/Src/driverSWPCAL6416.c \
				Drivers/SWDrivers/Src/driverSWSHT21.c \
				Drivers/SWDrivers/Src/driverSWSSD1306.c \
				Drivers/SWDrivers/Src/driverSWStorageManager.c \
				Drivers/SWDrivers/Src/driverSWUART2.c \
#$(wildcard Drivers/SWDrivers/Src/driver*.c)

SWDRIVER_INC =	Drivers/SWDrivers/Inc

HAL_SRC =	Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_cortex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_uart_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_can.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pwr.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_spi.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_spi_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_i2c_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash_ex.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_rcc.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_tim.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_flash.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_adc.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_gpio.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_dma.c \
			Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_iwdg.c \
#$(wildcard Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal*.c)

HAL_INC = 		Drivers/STM32F3xx_HAL_Driver/Inc \
				Drivers/CMSIS/Include/ \
				Drivers/CMSIS/Device/ST/STM32F3xx/Include 