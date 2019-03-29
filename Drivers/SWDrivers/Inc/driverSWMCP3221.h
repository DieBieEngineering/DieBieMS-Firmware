#include "stm32f3xx_hal.h"
#include "stdbool.h"
#include "math.h"
#include "modDelay.h"
#include "driverHWI2C1.h"

#define MCP3221_ADDRES	0x4D

void driverSWMCP3221Init(void);
uint16_t driverSWMCP3221GetValue(void);
float    driverSWMCP3221GetTemperature(uint32_t ntcNominal,uint32_t ntcSeriesResistance,uint16_t ntcBetaFactor, float ntcNominalTemp);
