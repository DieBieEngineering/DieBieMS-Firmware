#include "driverHWPowerState.h"

const PowerStatePortStruct driverHWPorts[NoOfPowersSTATs] = 		// Hold all status configuration data
{
	{GPIOB,RCC_AHBENR_GPIOAEN,GPIO_PIN_5,GPIO_MODE_OUTPUT_PP,GPIO_NOPULL},		// P_STAT_POWER_ENABLE
	{GPIOB,RCC_AHBENR_GPIOCEN,GPIO_PIN_4,GPIO_MODE_INPUT,GPIO_PULLUP},				// P_STAT_BUTTON_INPUT
	{GPIOA,RCC_AHBENR_GPIOCEN,GPIO_PIN_0,GPIO_MODE_INPUT,GPIO_PULLUP}					// P_STAT_CHARGE_DETECT
};

void driverHWPowerStateInit(void) {
	GPIO_InitTypeDef PowerStatePortHolder;
	uint8_t STATPointer;
	
	for(STATPointer = 0; STATPointer < NoOfPowersSTATs; STATPointer++) {
		RCC->AHBENR |= driverHWPorts[STATPointer].ClkRegister;								// Enable clock de desired port
		PowerStatePortHolder.Mode = driverHWPorts[STATPointer].Mode;					// Push pull output
		PowerStatePortHolder.Pin = driverHWPorts[STATPointer].Pin;						// Points to status pin
		PowerStatePortHolder.Pull = driverHWPorts[STATPointer].Pull;					// No pullup
		PowerStatePortHolder.Speed = GPIO_SPEED_HIGH;														// GPIO clock speed
		HAL_GPIO_Init(driverHWPorts[STATPointer].Port,&PowerStatePortHolder);	// Perform the IO init 
	};
};

void driverHWPowerStateSetOutput(PowerStateIDTypedef outputPort, PowerStateStateTypedef newState) {
	HAL_GPIO_WritePin(driverHWPorts[outputPort].Port,driverHWPorts[outputPort].Pin,(GPIO_PinState)newState); // Set desired pin to desired state
};

bool driverHWPowerStateReadInput(PowerStateIDTypedef inputPort) {	
	return (bool) HAL_GPIO_ReadPin(driverHWPorts[inputPort].Port,driverHWPorts[inputPort].Pin);
};
