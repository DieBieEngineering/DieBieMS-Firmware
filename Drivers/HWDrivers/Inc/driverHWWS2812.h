#ifndef __DRIVERHWWS2812
#define __DRIVERHWWS2812

#include "stm32f3xx_hal.h"
#include "stdbool.h"

typedef union {
	__packed struct{
		uint8_t  r;
		uint8_t  g;
		uint8_t  b;
		uint8_t  dummy;	// padding
	}color;
	uint32_t BGR;
} pixelData;

void driverHWWS2812Init(void);
void driverHWWS2812WriteByte(uint32_t byte);
void driverHWWS2812WritePixels(pixelData *colors, uint32_t count);

#endif
