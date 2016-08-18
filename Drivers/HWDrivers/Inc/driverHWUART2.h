#include "stm32f3xx_hal.h"
#include "stdbool.h"

extern FILE driverSWUART2IOStream;

void driverHWUART2Init(void (*receiveCallBack)(UART_HandleTypeDef*));
void driverHWUART2SendChar(uint8_t character);
uint8_t driverHWUART2GetChar(void);
