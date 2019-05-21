#include "stm32f3xx_hal.h"
#include "stdbool.h"
#include "math.h"
#include "modDelay.h"
#include "driverHWI2C1.h"

#define ADS1015_ADDRES 0x49

// Top resistor 10M bottom resistor 120k range +/- 2048mV

// Vin = conversionValue * 2.048/2^15*(1+10M/120K)

typedef enum {
	ADS1015RegConversion = 0,
  ADS1015RegConfig,
	ADS1015RegLowThreshold,
	ADS1015RegHighThreshold
} ADS1015RegistersEnum;

typedef enum {
	ADS1015P0N1 = 0,
	ADS1015P0N3,
	ADS1015P1N3,
	ADS1015P2N3,
	ADS1015P0NGND,
	ADS1015P1NGND,
	ADS1015P2NGND,
	ADS1015P3NGND
} ADS1015MuxSettingsEnum;

void    driverSWADS1015Init(void);
void    driverSWADS1015SetContSampleChannel(uint8_t MUX);
int16_t driverSWADS1015GetConversionData(void);
void    driverSWADS1015SampleTask(bool ADSADCPresent);
float   driverSWADS1015GetVoltage(uint8_t channel, float factor);
void    driverSWADS1015ResetValues(void);
