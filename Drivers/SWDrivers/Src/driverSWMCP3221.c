#include "driverSWMCP3221.h"

#define averageCount 20

uint16_t driverSWMCP3221MovingAverage[averageCount];

void driverSWMCP3221Init(void) {
	uint8_t averageSumCounter;
	
	for(averageSumCounter = 0; averageSumCounter < averageCount; averageSumCounter++) {
	  driverSWMCP3221MovingAverage[averageSumCounter] = driverSWMCP3221GetValue();
	}
}

uint16_t driverSWMCP3221GetValue(void){
	uint8_t readBytes[2];

	driverHWI2C1Read(MCP3221_ADDRES,readBytes,2);
	
	return (readBytes[0] << 8) | (readBytes[1]);
}

float driverSWMCP3221GetTemperature(uint32_t ntcNominal,uint32_t ntcSeriesResistance,uint16_t ntcBetaFactor, float ntcNominalTemp){
	static uint8_t averagePointer;	
	uint32_t adcValue = 0;
	float scalar;
	float steinhart;
	uint32_t averageSum = 0;
	uint8_t averageSumCounter;
	
	averagePointer = averagePointer % averageCount;
	
	driverSWMCP3221MovingAverage[averagePointer++] = driverSWMCP3221GetValue();
	
	for(averageSumCounter = 0; averageSumCounter < averageCount; averageSumCounter++) {
	  averageSum += driverSWMCP3221MovingAverage[averageSumCounter];
	}
	
	adcValue = averageSum/averageCount;

  scalar = 4095.0f / (float)adcValue - 1.0f;
  scalar = (float)ntcSeriesResistance / scalar;
  steinhart = scalar / (float)ntcNominal;               // (R/Ro)
  steinhart = log(steinhart);                           // ln(R/Ro)
  steinhart /= (float)ntcBetaFactor;                    // 1/B * ln(R/Ro)
  steinhart += 1.0f / (ntcNominalTemp + 273.15f);       // + (1/To)
  steinhart = 1.0f / steinhart;                         // Invert
  steinhart -= 273.15f;                                 // convert to degree
	
	if(steinhart < -100.0f)
		steinhart = 200;
	
  return steinhart;
}
