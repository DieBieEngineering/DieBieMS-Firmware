#include "stm32f3xx_hal.h"
#include "stdbool.h"
#include "stdio.h"
#include "driverHWUART2.h"

void driverSWUART2Init(void);
void driverSWUART2Printf(const char *format, ...);
void driverSWUART2Task(void);
