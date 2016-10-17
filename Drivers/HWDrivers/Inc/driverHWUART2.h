#include "stm32f3xx_hal.h"
#include "stdbool.h"
#include "string.h"

#define RX_BUFFER_SIZE					64

extern FILE driverSWUART2IOStream;

void driverHWUART2Init(void);
void driverHWUART2SendChar(uint8_t character);
bool driverHWUART2GetChar(char *character);
