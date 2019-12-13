VPATH += Drivers/HWDrivers/Src/ \
		 Drivers/SWDrivers/Src/ \
		 Drivers/STM32F3xx_HAL_Driver/Src/ \

HWDRIVER_SRC = 	$(wildcard Drivers/HWDrivers/Src/driver*.c)

HWDRIVER_INC =	Drivers/HWDrivers/Inc

SWDRIVER_SRC =	$(wildcard Drivers/SWDrivers/Src/driver*.c)

SWDRIVER_INC =	Drivers/SWDrivers/Inc

HAL_SRC =	$(wildcard Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal*.c)

HAL_INC = 		Drivers/STM32F3xx_HAL_Driver/Inc \
				Drivers/CMSIS/Include/ \
				Drivers/CMSIS/Device/ST/STM32F3xx/Include 