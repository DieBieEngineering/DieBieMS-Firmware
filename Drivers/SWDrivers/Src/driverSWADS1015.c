#include "driverSWADS1015.h"

uint32_t driverSWADS1015SampleLastTick;
int16_t  driverSWADS1015Channel01        = 0;
int16_t  driverSWADS1015Channel23        = 0;
bool     driverSWADS1015ChannelToggle    = false;

float battVoltage;

void driverSWADS1015Init(void) {
	driverSWADS1015SampleLastTick = HAL_GetTick();
	
	driverSWADS1015SetContSampleChannel(0x00);
}

void driverSWADS1015SetContSampleChannel(uint8_t MUX) {
	uint8_t MUXFiltered = 0x07 & MUX;
	uint8_t configMSB   = (MUXFiltered << 4) + 0x04;
	uint8_t configLSB   = 0x80;
	
	uint8_t configBytes[] = {ADS1015RegConfig,configMSB,configLSB};
	driverHWI2C1Write(ADS1015_ADDRES,false,configBytes,sizeof(configBytes));
}

int16_t driverSWADS1015GetConversionData(void) {
  int16_t conversionResult;
	
	uint8_t conversionRegister = ADS1015RegConversion;
	uint8_t readBytes[2];
	driverHWI2C1Write(ADS1015_ADDRES,false,&conversionRegister,1);
	driverHWI2C1Read(ADS1015_ADDRES,readBytes,2);
	
	conversionResult = (readBytes[0] << 8) + readBytes[1];
	
	return conversionResult;
}

void driverSWADS1015SampleTask(bool ADSADCPresent) {
  if(modDelayTick1ms(&driverSWADS1015SampleLastTick,20)) {
		if(ADSADCPresent) {
		if(driverSWADS1015ChannelToggle) {																													// Alternate between channels
			driverSWADS1015Channel23 = driverSWADS1015GetConversionData();                            // Get latest sample
			driverSWADS1015SetContSampleChannel(ADS1015P0N1);
		}else{
		  driverSWADS1015Channel01 = driverSWADS1015GetConversionData();														// Get latest sample
			driverSWADS1015SetContSampleChannel(ADS1015P2N3);
		}
		
		driverSWADS1015ChannelToggle ^= true;																												// Toggle channel pointer
		}else{
			driverSWADS1015ResetValues();
		}
	}
}

float driverSWADS1015GetVoltage(uint8_t channel, float factor) {
	float returnValue = 0.0f;
	switch (channel) {
	  case ADS1015P0N1:
			returnValue = (float)factor*driverSWADS1015Channel01;
			break;
	  case ADS1015P2N3:
			returnValue = (float)factor*driverSWADS1015Channel23;
			break;
		default:
			break;
	}
	return returnValue;
}

void driverSWADS1015ResetValues(void) {
	driverSWADS1015Channel01        = 0;
	driverSWADS1015Channel23        = 0;
	driverSWADS1015ChannelToggle    = false;
}

