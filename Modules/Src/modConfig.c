#include "modConfig.h"

modConfigGeneralConfigStructTypedef modConfigGeneralConfig;

uint16_t *driverHWEEPROMVirtAddVarTab;
uint32_t ee_status;

uint16_t modConfigStructSize = (sizeof(modConfigGeneralConfigStructTypedef)/sizeof(uint16_t));// Calculate the space needed in EEPROM;

modConfigGeneralConfigStructTypedef* modConfigInit(void) {
	// Generate EEPROM enviroment
	driverHWEEPROMVirtAddVarTab = malloc(modConfigStructSize);															// Make room for the addres array
	HAL_FLASH_Unlock();																																			// Unlock FLASH to allow EEPROM lib to write
	if(driverHWEEPROMInit(modConfigStructSize) != HAL_OK)																		// Init EEPROM and tell EEPROM manager the disered size
		while(true);																																					// Something went wrong in the init
	
	for(uint16_t addresPointer=0 ; addresPointer<modConfigStructSize ; addresPointer++)
		driverHWEEPROMVirtAddVarTab[addresPointer] = addresPointer;														// Generate addres array
	
	// Check EEPROM Contents
	uint16_t readVariable;																																	// Define variable to write EEPROM contents to if any
	if(driverHWEEPROMReadVariable(driverHWEEPROMVirtAddVarTab[0], &readVariable) == 1)			// Try to read from first memory location
		modConfigStoreDefaultConfig();																												// Error, EEPROM Empty, write the default information to EEPROM
	
	// Read config from EEPROM
	modConfigLoadConfig();																																	// EEPROM contains info, read EEPROM contents to config variable
	
	return &modConfigGeneralConfig;
};

bool modConfigStoreDefaultConfig(void) {
	modConfigGeneralConfigStructTypedef defaultConfig;
	defaultConfig.noOfCells 																=	6;														// 12 Cells in series
	defaultConfig.cellHardUnderVoltage											= 2.80f;												// Worst case 2.8V as lowest cell voltage
	defaultConfig.cellHardOverVoltage												= 4.30f;												// Worst case 4.2V as highest cell voltage
	defaultConfig.cellSoftUnderVoltage											= 3.00f;												// Normal lowest cell voltage 3V
	defaultConfig.cellSoftOverVoltage												= 4.15f;												// Normal highest cell voltage 4.15V
	defaultConfig.cellBalanceDifferenceThreshold						=	0.010f;												// Start balancing @ 10mV difference
	defaultConfig.cellBalanceStart													= 3.80f;												// Start balancing above 3.9V
	defaultConfig.cellBalanceUpdateInterval									= 4*1000;												// Keep calculated resistors enabled for this amount of time in miliseconds
	defaultConfig.maxSimultaneousDischargingCells						= 5;														// Allow a maximum of 5 cells simultinous discharging trough bleeding resistors
	defaultConfig.timoutDischargeRetry											= 10*1000;											// Wait for 10 seconds before retrying to enable load.
	defaultConfig.hysteresisDischarge 											= 0.02f;												// Lowest cell should rise 20mV before output is re enabled
	defaultConfig.timoutChargeRetry													= 30*1000;											// Wait for 30 seconds before retrying to enable charger
	defaultConfig.hysteresisCharge													= 0.01f;												// Highest cell should lower 10mV before input is re enabled
	defaultConfig.timoutChargeCompleted											= 30*60*1000;										// Wait for 30 minutes before setting charge state to charged
	defaultConfig.timoutChargingCompletedMinimalMismatch 		= 1*60*1000;										// If cell mismatch is under threshold and charging is not allowed timout this delay to determin charged state
	defaultConfig.maxMismatchThreshold											= 0.04f;												// If mismatch is under this threshold for timoutChargingCompletedMinimalMismatch determin charged
	defaultConfig.chargerEnabledThreshold										= 0.2f;													// If charge current > 0.2A stay in charging mode and dont power off
	defaultConfig.timoutChargerDisconnected									= 2000;													// Wait for 2 seconds to respond to charger disconnect
	defaultConfig.minimalPrechargePercentage								= 0.80f;												// output should be at a minimal of 85% of input voltage
	defaultConfig.timoutPreCharge														= 300;													// Precharge error timout
	defaultConfig.maxAllowedCurrent													= 70.0f;												// Allow max 70A trough BMS
	defaultConfig.displayTimoutBatteryDead									= 5000;													// Show battery dead symbol 5 seconds before going to powerdown
	defaultConfig.displayTimoutBatteryError									= 2000;													// Shot error symbol for 2 seconds before going to powerdown
	defaultConfig.displayTimoutSplashScreen									=	1000;													// Display / INIT time
	defaultConfig.maxUnderAndOverVoltageErrorCount 					= 5;														// Max count of hard cell voltage errors
	defaultConfig.notUsedCurrentThreshold										= 0.5f;													// If abs(packcurrent) < 500mA consider pack as not used
	defaultConfig.notUsedTimout															= 30*60*1000;										// If pack is not used for longer than 30 minutes disable pack
	
	uint16_t *dataPointer = (uint16_t*)&defaultConfig;																			// Trick to convert struckt to a 16 bit pointer.
	
	for(uint16_t addresPointer=0 ; addresPointer<modConfigStructSize ; addresPointer++)			// Pass trough all adresses
		driverHWEEPROMWriteVariable(driverHWEEPROMVirtAddVarTab[addresPointer],dataPointer[addresPointer]);	// Store data in EEPROM
	
	return false;
};

bool modConfigStoreConfig(void) {
	uint16_t *dataPointer = (uint16_t*)&modConfigGeneralConfig;															// Trick to convert struct to a 16 bit pointer.
	
	for(uint16_t addresPointer=0 ; addresPointer<modConfigStructSize ; addresPointer++)			// Pass trough all adresses
		driverHWEEPROMWriteVariable(driverHWEEPROMVirtAddVarTab[addresPointer],dataPointer[addresPointer]);	// Store data in EEPROM
	
	return false;
};

bool modConfigLoadConfig(void) {
	uint16_t *dataPointer = (uint16_t*)&modConfigGeneralConfig;															// Trick to convert struct to a 16 bit pointer.
	
	for(uint16_t addresPointer=0 ; addresPointer<modConfigStructSize ; addresPointer++)
		driverHWEEPROMReadVariable(driverHWEEPROMVirtAddVarTab[addresPointer],&dataPointer[addresPointer]);

	
	/* The config should be loaded from memory here (instead of statically defined):
	modConfigGeneralConfig.noOfCells 																=	6;										// 12 Cells in series
	modConfigGeneralConfig.cellHardUnderVoltage											= 2.80f;								// Worst case 2.8V as lowest cell voltage
	modConfigGeneralConfig.cellHardOverVoltage											= 4.30f;								// Worst case 4.2V as highest cell voltage
	modConfigGeneralConfig.cellSoftUnderVoltage											= 3.00f;								// Normal lowest cell voltage 3V
	modConfigGeneralConfig.cellSoftOverVoltage											= 4.15f;								// Normal highest cell voltage 4.1V
	modConfigGeneralConfig.cellBalanceDifferenceThreshold						=	0.010f;								// Start balancing @ 10mV difference
	modConfigGeneralConfig.cellBalanceStart													= 3.80f;								// Start balancing above 3.9V
	modConfigGeneralConfig.cellBalanceUpdateInterval								= 4*1000;								// Keep calculated resistors enabled for this amount of time in miliseconds
	modConfigGeneralConfig.maxSimultaneousDischargingCells					= 5;										// Allow a maximum of 5 cells simultinous discharging trough bleeding resistors
	modConfigGeneralConfig.timoutDischargeRetry											= 10*1000;							// Wait for 10 seconds before retrying to enable load.
	modConfigGeneralConfig.hysteresisDischarge 											= 0.02f;								// Lowest cell should rise 20mV before output is re enabled
	modConfigGeneralConfig.timoutChargeRetry												= 30*1000;							// Wait for 30 seconds before retrying to enable charger
	modConfigGeneralConfig.hysteresisCharge													= 0.01f;								// Highest cell should lower 10mV before input is re enabled
	modConfigGeneralConfig.timoutChargeCompleted										= 30*60*1000;						// Wait for 30 minutes before setting charge state to charged
	modConfigGeneralConfig.timoutChargingCompletedMinimalMismatch 	= 1*60*1000;						// If cell mismatch is under threshold and charging is not allowed timout this delay to determin charged state
	modConfigGeneralConfig.maxMismatchThreshold											= 0.02f;								// If mismatch is under this threshold for timoutChargingCompletedMinimalMismatch determin charged
	modConfigGeneralConfig.chargerEnabledThreshold									= 0.2f;									// If charge current > 0.2A stay in charging mode and dont power off
	modConfigGeneralConfig.timoutChargerDisconnected								= 2000;									// Wait for 2 seconds to detect charger disconnect
	modConfigGeneralConfig.minimalPrechargePercentage								= 0.80f;								// output should be at a minimal of 85% of input voltage
	modConfigGeneralConfig.timoutPreCharge													= 300;									// Precharge error timout
	modConfigGeneralConfig.maxAllowedCurrent												= 70.0f;								// Allow max 70A trough BMS
	modConfigGeneralConfig.displayTimoutBatteryDead									= 5000;									// Show battery dead symbol 5 seconds before going to powerdown
	modConfigGeneralConfig.displayTimoutSplashScreen								=	1000;									// Display / INIT time
	modConfigGeneralConfig.maxUnderAndOverVoltageErrorCount 				= 5;										// Max count of hard cell voltage errors
	*/
	return false;
};
