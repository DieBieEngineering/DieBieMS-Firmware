#include "stm32f3xx_hal.h"
#include "stdbool.h"

#define NoOfADCPorts 1
#define NoOfAverages 40

typedef struct {
	GPIO_TypeDef* Port;
	uint32_t ClkRegister;
	uint32_t Pin;
	uint32_t Mode;
	uint32_t Pull;
	uint32_t Alternate;
} driverHWADCPortStruct;

void driverHWADCInit(void);
bool driverHWADCGetLoadVoltage(float *loadVoltage);
