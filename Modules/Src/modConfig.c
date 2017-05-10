#include "modConfig.h"

modConfigGeneralConfigStructTypedef modConfigGeneralConfig;

modConfigGeneralConfigStructTypedef* modConfigInit(void) {
	driverSWStorageManagerConfigStructSize = (sizeof(modConfigGeneralConfigStructTypedef)/sizeof(uint16_t)); // Calculate the space needed for the config struct in EEPROM
	return &modConfigGeneralConfig;
};

bool modConfigStoreAndLoadDefaultConfig(void) {
	bool returnVal = false;
	if(driverSWStorageManagerConfigEmpty) {
		// VALUES WILL ONLY UPDATE AFTER FLASH ERASE!
		modConfigGeneralConfigStructTypedef defaultConfig;
		defaultConfig.noOfCells 																=	12;														// 12 Cells in series
		defaultConfig.batteryCapacity														=	10.00f;												// 10Ah battery
		defaultConfig.cellHardUnderVoltage											= 2.40f;												// Worst case 2.4V as lowest cell voltage
		defaultConfig.cellHardOverVoltage												= 4.45f;												// Worst case 4.35V as highest cell voltage
		defaultConfig.cellSoftUnderVoltage											= 2.60f;												// Normal lowest cell voltage 2.6V
		defaultConfig.cellSoftOverVoltage												= 4.20f;												// Normal highest cell voltage 4.15V
		defaultConfig.cellBalanceDifferenceThreshold						=	0.001f;												// Start balancing @ 5mV difference, stop if below
		defaultConfig.cellBalanceStart													= 3.80f;												// Start balancing above 3.8V
		defaultConfig.cellBalanceUpdateInterval									= 4*1000;												// Keep calculated resistors enabled for this amount of time in miliseconds
		defaultConfig.maxSimultaneousDischargingCells						= 5;														// Allow a maximum of 5 cells simultinous discharging trough bleeding resistors
		defaultConfig.timoutDischargeRetry											= 4*1000;												// Wait for 10 seconds before retrying to enable load.
		defaultConfig.hysteresisDischarge 											= 0.02f;												// Lowest cell should rise 20mV before output is re enabled
		defaultConfig.timoutChargeRetry													= 30*1000;											// Wait for 30 seconds before retrying to enable charger
		defaultConfig.hysteresisCharge													= 0.01f;												// Highest cell should lower 10mV before input is re enabled
		defaultConfig.timoutChargeCompleted											= 30*60*1000;										// Wait for 30 minutes before setting charge state to charged
		defaultConfig.timoutChargingCompletedMinimalMismatch 		= 6*1000;												// If cell mismatch is under threshold and (charging is not allowed) wait this delay time to set "charged" state
		defaultConfig.maxMismatchThreshold											= 0.020f;												// If mismatch is under this threshold for timoutChargingCompletedMinimalMismatch determin fully charged
		defaultConfig.chargerEnabledThreshold										= 0.2f;													// If charge current > 0.2A stay in charging mode and dont power off
		defaultConfig.timoutChargerDisconnected									= 2000;													// Wait for 2 seconds to respond to charger disconnect
		defaultConfig.minimalPrechargePercentage								= 0.80f;												// output should be at a minimal of 80% of input voltage
		defaultConfig.timoutPreCharge														= 300;													// Precharge error timout, allow 300ms pre-charge time before declaring load error
		defaultConfig.maxAllowedCurrent													= 120.0f;												// Allow max 120A trough BMS
		defaultConfig.displayTimoutBatteryDead									= 5000;													// Show battery dead symbol 5 seconds before going to powerdown in cell voltage error state
		defaultConfig.displayTimoutBatteryError									= 2000;													// Show error symbol for 2 seconds before going to powerdown in general error state
		defaultConfig.displayTimoutSplashScreen									=	1000;													// Display / INIT splash screen time
		defaultConfig.maxUnderAndOverVoltageErrorCount 					= 5;														// Max count of hard cell voltage errors
		defaultConfig.notUsedCurrentThreshold										= 0.5f;													// If abs(packcurrent) < 500mA consider pack as not used
		defaultConfig.notUsedTimout															= 30*60*1000;										// If pack is not used for longer than 30 minutes disable bms
		defaultConfig.stateOfChargeStoreInterval								= 60*1000;											// Interval in ms to store state of charge information
		
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
