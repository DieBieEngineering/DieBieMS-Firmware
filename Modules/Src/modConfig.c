#include "modConfig.h"

modConfigGeneralConfigStructTypedef modConfigGeneralConfig;

modConfigGeneralConfigStructTypedef* modConfigInit(void) {
	driverSWStorageManagerConfigStructSize = (sizeof(modConfigGeneralConfigStructTypedef)/sizeof(uint16_t)); // Calculate the space needed for the config struct in EEPROM
	return &modConfigGeneralConfig;
};

bool modConfigStoreAndLoadDefaultConfig(void) {
	bool returnVal = false;
	if(driverSWStorageManagerConfigEmpty) {
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
		
		driverSWStorageManagerConfigEmpty = false;
		returnVal = driverSWStorageManagerStoreConfigStruct(&defaultConfig,STORAGE_CONFIG);
	}
	
	modConfigLoadConfig();
	return returnVal;
};

bool modConfigStoreConfig(void) {
	return driverSWStorageManagerStoreConfigStruct(&modConfigGeneralConfig,STORAGE_CONFIG);
};

bool modConfigLoadConfig(void) {
	return driverSWStorageManagerGetConfigStruct(&modConfigGeneralConfig,STORAGE_CONFIG);
};
