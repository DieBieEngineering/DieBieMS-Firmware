#include "modConfig.h"

modConfigGeneralConfigStructTypedef modConfigGeneralConfig;

void modConfigInit(modConfigGeneralConfigStructTypedef **configStructPointer) {
	*configStructPointer = &modConfigGeneralConfig;																	// Fill the pointer with the addres to the modConfigGeneralConfig to be used in the above code
	
	modConfigLoadConfig();																													// Load data from emulated EEPROM
};

bool modConfigStoreConfig(void) {
	return false;
};

bool modConfigLoadConfig(void) {
	// The config should be loaded from memory here (instead of statically defined):
	modConfigGeneralConfig.noOfCells 												=	 12;									// 12 Cells in series
	modConfigGeneralConfig.cellHardUnderVoltage							= 2.80f;								// Worst case 2.8V as lowest cell voltage
	modConfigGeneralConfig.cellHardOverVoltage							= 4.20f;								// Worst case 4.2V as highest cell voltage
	modConfigGeneralConfig.cellSoftUnderVoltage							= 3.00f;								// Normal lowest cell voltage 3V
	modConfigGeneralConfig.cellSoftOverVoltage							= 4.10f;								// Normal highest cell voltage 4.1V
	modConfigGeneralConfig.cellBalanceDifferenceThreshold		=	0.05f;								// Start balancing @ 50mV difference
	modConfigGeneralConfig.cellBalanceStart									= 3.90f;								// Start balancing above 3.9V
	modConfigGeneralConfig.maxSimultaneousDischargingCells	= 6;										// Allow a maximum of 6 cells simultinous discharging
	modConfigGeneralConfig.timoutDischargeRetry							= 10*1000;							// Wait for 10 seconds before retrying to enable load.
	modConfigGeneralConfig.hysteresisDischarge 							= 20.0f;								// Lowest cell should rise 20mV before output is re enabled
	modConfigGeneralConfig.timoutChargeRetry								= 10*1000;							// Wait for 10 seconds before retrying to enable charger
	modConfigGeneralConfig.hysteresisCharge									= 10.0f;								// Highest cell should lower 10mV before input is re enabled
	modConfigGeneralConfig.timoutChargeCompleted						= 30*60*1000;						// Wait for 30 minutes before setting charge state to charged
	modConfigGeneralConfig.minimalPrechargePercentage				= 0.85f;								// output should be at a minimal of 85% of input voltage
	modConfigGeneralConfig.maxAllowedCurrent								= 70.0f;								// Allow max 70A trough BMS
	
	return false;
};
