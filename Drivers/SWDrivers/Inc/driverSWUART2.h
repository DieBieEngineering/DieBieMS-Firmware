#include "stm32f3xx_hal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include "driverHWUART2.h"
#include "libRingBuffer.h"
#include "libFileStream.h"

#define RINGBUFFERSIZE									50

void driverSWUART2Init(void);
char driverSWUART2PutCharInOutputBuffer(char character, FILE *stream);
void driverSWUART2Task(void);
