#include "modConfig.h"

modConfigGeneralConfigStructTypedef modConfigGeneralConfig;

modConfigGeneralConfigStructTypedef* modConfigInit(void) {
	driverSWStorageManagerConfigStructSize = (sizeof(modConfigGeneralConfigStructTypedef)/sizeof(uint16_t)); // Calculate the space needed for the config struct in EEPROM
	return &modConfigGeneralConfig;
};

bool modConfigStoreAndLoadDefaultConfig(void) {
	bool returnVal = false;
	if(driverSWStorageManagerConfigEmpty) {
		returnVal = modConfigStoreDefaultConfig();
	}
	
	modConfigLoadConfig();
	return returnVal;
};

bool modConfigStoreConfig(void) {
	return driverSWStorageManagerStoreStruct(&modConfigGeneralConfig,STORAGE_CONFIG);
	// TODO_EEPROM
};

bool modConfigLoadConfig(void) {
	return driverSWStorageManagerGetStruct(&modConfigGeneralConfig,STORAGE_CONFIG);
};

bool modConfigStoreDefaultConfig(void) {
	// VALUES WILL ONLY AUTIMATICALLY UPDATE AFTER FLASH ERASE!
	// Master BMS
	modConfigGeneralConfigStructTypedef defaultConfig;
	defaultConfig.noOfCells 																     = 12;											// X Cells in series
	defaultConfig.batteryCapacity														     = 40.00f;									  // XXAh battery
	defaultConfig.cellHardUnderVoltage											     = 2.30f;										// Worst case X.XXV as lowest cell voltage
	defaultConfig.cellHardOverVoltage												     = 4.25f;										// Worst case X.XXV as highest cell voltage
	defaultConfig.cellLCSoftUnderVoltage											   = 2.50f;										// Low current lowest cell voltage X.XXV.
  defaultConfig.cellHCSoftUnderVoltage                         = 3.00f;                   // High current lowest cell voltage X.XXV.
	defaultConfig.cellSoftOverVoltage												     = 4.15f;										// Normal highest cell voltage X.XXV.
	defaultConfig.cellBalanceDifferenceThreshold                 = 0.01f;										// Start balancing @ XmV difference, stop if below.
	defaultConfig.cellBalanceStart													     = 3.80f;										// Start balancing above X.XXV.
	defaultConfig.cellThrottleUpperStart										     = 0.03f;										// Upper range of cell voltage for charge throttling.
	defaultConfig.cellThrottleLowerStart										     = 0.20f;									  // Lower range of cell voltage for discharge throttling.
	defaultConfig.cellThrottleUpperMargin										     = 0.01f;										// Margin of throttle from upper soft limits.
	defaultConfig.cellThrottleLowerMargin										     = 0.50f;									  // Margin of throttle from lower soft limits.	
	defaultConfig.throttleChargeIncreaseRate                     = 1;                       // Percentage charge throttle increase rate per 100ms (cell voltage loop time)  
	defaultConfig.throttleDisChargeIncreaseRate                  = 2;                       // Percentage discharge throttle increase rate per 100ms (cell voltage loop time)  	
	defaultConfig.cellBalanceUpdateInterval									     = 4*1000;									// Keep calculated resistors enabled for this amount of time in miliseconds.
	defaultConfig.maxSimultaneousDischargingCells						     = 5;												// Allow a maximum of X cells simultinous discharging trough bleeding resistors.
	defaultConfig.timeoutDischargeRetry											     = 4*1000;									// Wait for X seconds before retrying to enable load.
	defaultConfig.hysteresisDischarge 											     = 0.02f;										// Lowest cell should rise XXmV before output is re enabled.
	defaultConfig.timeoutChargeRetry												     = 30*1000;									// Wait for XX seconds before retrying to enable charger.
	defaultConfig.hysteresisCharge													     = 0.01f;										// Highest cell should lower XXmV before charger is re enabled.
	defaultConfig.timeoutChargeCompleted										     = 30*60*1000;							// Wait for XX minutes before setting charge state to charged.
	defaultConfig.timeoutChargingCompletedMinimalMismatch 	     = 6*1000;									// If cell mismatch is under threshold and (charging is not allowed) wait this delay time to set "charged" state.
	defaultConfig.maxMismatchThreshold											     = 0.020f;									// If mismatch is under this threshold for timeoutChargingCompletedMinimalMismatch determin fully charged.
	defaultConfig.chargerEnabledThreshold										     = 0.5f;										// If charge current > X.XA stay in charging mode and dont power off.
	defaultConfig.timeoutChargerDisconnected								     = 2000;										// Wait for X seconds to respond to charger disconnect.
	defaultConfig.minimalPrechargePercentage								     = 0.80f;										// output should be at a minimal of 80% of input voltage.
	defaultConfig.timeoutLCPreCharge												     = 300;											// Precharge error timeout, allow 300ms pre-charge time before declaring load error.
	defaultConfig.maxAllowedCurrent													     = 120.0f;									// Allow max XXXA trough BMS.
	defaultConfig.displayTimeoutBatteryDead									     = 5000;										// Show battery dead symbol X seconds before going to powerdown in cell voltage error state.
	defaultConfig.displayTimeoutBatteryError								     = 2000;										// Show error symbol for X seconds before going to powerdown in general error state.
	defaultConfig.displayTimeoutBatteryErrorPreCharge				     = 10000;										// Show pre charge error for XX seconds.
	defaultConfig.displayTimeoutSplashScreen								     = 1000;										// Display / INIT splash screen time.
	defaultConfig.maxUnderAndOverVoltageErrorCount 					     = 5;												// Max count of hard cell voltage errors.
	defaultConfig.notUsedCurrentThreshold										     = 1.0f;										// If abs(packcurrent) < X.XA consider pack as not used.
	defaultConfig.notUsedTimeout														     = 60*60*1000;							// If pack is not used for longer than XX minutes disable bms.
	defaultConfig.stateOfChargeStoreInterval								     = 60*1000;									// Interval in ms to store state of charge information.
	defaultConfig.CANID																			     = 8;											// CAN ID for CAN communication.
	defaultConfig.CANIDStyle                                     = CANIDStyleFoiler;        // CAN ID default Style
	defaultConfig.tempEnableMaskBMS                              = 0x1C0F;									// Bitwise select what sensor to enable for the BMS (internal sensors).
	defaultConfig.tempEnableMaskBattery                          = 0x03F0;									// Bitwise select what sensor to enable for the battery (external sensors).
  defaultConfig.LCUseDischarge                                 = true;                    // Enable or disable the solid state output
	defaultConfig.LCUsePrecharge                                 = true;                    // Use precharge before enabling main output
	defaultConfig.NTCTopResistor[modConfigNTCGroupLTCExt]        = 100000;                  // NTC Pullup resistor value
	defaultConfig.NTCTopResistor[modConfigNTCGroupMasterPCB]     = 100000;                  // NTC Pullup resistor value
	defaultConfig.NTC25DegResistance[modConfigNTCGroupLTCExt]    = 100000;                  // NTC resistance at 25 degree
	defaultConfig.NTC25DegResistance[modConfigNTCGroupMasterPCB] = 100000;                  // NTC resistance at 25 degree
	defaultConfig.NTCBetaFactor[modConfigNTCGroupLTCExt]         = 4390;                    // NTC Beta factor
	defaultConfig.NTCBetaFactor[modConfigNTCGroupMasterPCB]      = 3590;                    // NTC Beta factor
	defaultConfig.allowChargingDuringDischarge                   = true;                    // Allow the battery to be charged in normal mode
	defaultConfig.allowForceOn                                   = false;                   // Allow the BMS to be forced ON by long actuation of the power button
	defaultConfig.pulseToggleButton                              = false;                   // Select either pulse or toggle power button
	defaultConfig.togglePowerModeDirectHCDelay                   = true;                    // Select either direct power state control or HC output control with delayed turn off.
	defaultConfig.useCANSafetyInput                              = true;                    // Use the safety input status from CAN
	defaultConfig.useCANDelayedPowerDown                         = true;                    // Use delayed power down
	
	// Slave - HiAmp Config
	defaultConfig.NTCTopResistor[modConfigNTCGroupHiAmpExt]      = 100000;                  // NTC Pullup resistor value
	defaultConfig.NTCTopResistor[modConfigNTCGroupHiAmpPCB]      = 100000;                  // NTC Pullup resistor value
	defaultConfig.NTC25DegResistance[modConfigNTCGroupHiAmpExt]  = 100000;                  // NTC resistance at 25 degree
	defaultConfig.NTC25DegResistance[modConfigNTCGroupHiAmpPCB]  = 100000;                  // NTC resistance at 25 degree
	defaultConfig.NTCBetaFactor[modConfigNTCGroupHiAmpExt]       = 4390;                    // NTC Beta factor
	defaultConfig.NTCBetaFactor[modConfigNTCGroupHiAmpPCB]       = 3590;                    // NTC Beta factor
	defaultConfig.HCUseRelay                                     = false;                    // Enable or disable the relay output, when false will also disable HC pre charge.
	defaultConfig.HCUsePrecharge                                 = true;                    // choice whether to precharge or not, will only work when HCUseRelay = true.
	defaultConfig.timeoutHCPreCharge													   = 300;											// Precharge error timeout, allow xxxms pre-charge time before declaring load error.
	defaultConfig.timeoutHCPreChargeRetryInterval						     = 20000;										// When pre charge failes wait this long in ms
	defaultConfig.timeoutHCRelayOverlap											     = 1000;										// When precharge succeeds enable both relay and precharge combined for this time, then go to relay only.
	
	driverSWStorageManagerConfigEmpty = false;
	return driverSWStorageManagerStoreStruct(&defaultConfig,STORAGE_CONFIG);
	// TODO_EEPROM
}
