VPATH += Drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/ \
		Drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc/ \
		Device/ \

		 

DEVICE_SRC = 	Drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/system_stm32f3xx.c \
				Device/stm32f3xx_hal_msp.c \
				Device/stm32f3xx_it.c \

DEVICE_INC = 	Drivers/CMSIS/Device/ST/STM32F3xx/Include

STARTUPASM =	Drivers/CMSIS/Device/ST/STM32F3xx/Source/Templates/gcc/startup_stm32f303xc.s