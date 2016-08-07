#include "driverHWADC.h"

// Example: http://visualgdb.com/tutorials/arm/stm32/adc/

ADC_HandleTypeDef hadc1;
uint32_t driverHWADCAverageSum = 0;
uint8_t	driverHWADCAverageCount = 1;

const driverHWADCPortStruct driverHWADCPorts[NoOfADCPorts] = 												// Hold all I2C pin configuration data
{
	{GPIOA,RCC_AHBENR_GPIOAEN,GPIO_PIN_1,GPIO_MODE_ANALOG,GPIO_NOPULL,0x00}		// LoadVoltageSense analog pin
};

void driverHWADCInit(void) {
	GPIO_InitTypeDef PortInitHolder;
	uint8_t PortPointer;
	
	for(PortPointer = 0; PortPointer < NoOfADCPorts; PortPointer++) {
		RCC->AHBENR |= driverHWADCPorts[PortPointer].ClkRegister;								// Enable clock de desired port
		PortInitHolder.Mode = driverHWADCPorts[PortPointer].Mode;								// Push pull output
		PortInitHolder.Pin = driverHWADCPorts[PortPointer].Pin;									// Points to status pin
		PortInitHolder.Pull = driverHWADCPorts[PortPointer].Pull;								// No pullup
		PortInitHolder.Speed = GPIO_SPEED_HIGH;																	// GPIO clock speed
		PortInitHolder.Alternate = driverHWADCPorts[PortPointer].Alternate;			// Alternate function
		HAL_GPIO_Init(driverHWADCPorts[PortPointer].Port,&PortInitHolder);			// Perform the IO init 
	};
	
	__ADC1_CLK_ENABLE();																											// Enable clock to ADC1

  ADC_ChannelConfTypeDef sConfig;

	// ADC config
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = DISABLE;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    while(true) {}; 																												// Error situation 
  }

	// Channel config
	sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_181CYCLES_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    while(true) {}; 																												// Error situation 
  }
	
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;																				// Enable clock de desired port
	PortInitHolder.Mode = GPIO_MODE_OUTPUT_PP;																// Push pull output
	PortInitHolder.Pin = GPIO_PIN_2;																					// Points to status pin
	PortInitHolder.Pull = GPIO_NOPULL;																				// No pullup
	PortInitHolder.Speed = GPIO_SPEED_HIGH;																		// GPIO clock speed
	PortInitHolder.Alternate = 0x00;																					// Alternate function
	HAL_GPIO_Init(GPIOB,&PortInitHolder);																			// Perform the IO init
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);
};

bool driverHWADCGetLoadVoltage(float *loadVoltage) {
	HAL_ADC_Start(&hadc1);
	
	driverHWADCAverageSum = 0;
	for(driverHWADCAverageCount = 0; driverHWADCAverageCount < NoOfAverages; driverHWADCAverageCount++) {
		driverHWADCAverageSum += HAL_ADC_GetValue(&hadc1);
	};
	
	uint16_t temp = driverHWADCAverageSum/NoOfAverages;
	*loadVoltage = temp*(3.3f/4096*17.4f);
	
	/*
	if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK) {
		driverHWADCAverageSum += HAL_ADC_GetValue(&hadc1);
		driverHWADCAverageCount++;
	};
	
	driverHWADCAverageCount %= NoOfAverages;
	
	if(!driverHWADCAverageCount){
		uint16_t temp = driverHWADCAverageSum/NoOfAverages;
		*loadVoltage = temp*(3.3f/4096*17.4f);
		driverHWADCAverageSum = 0;
	}
	*/
	
	/*
	if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK) {
		volatile uint16_t temp = HAL_ADC_GetValue(&hadc1);
		*loadVoltage = temp*(3.3f/4096*17.4f);
	}
	*/
	return false;
};
