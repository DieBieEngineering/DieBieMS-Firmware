#ifndef __MODCONFIG_H
#define __MODCONFIG_H

#include "stdint.h"
#include "stdbool.h"
#include "driverSWStorageManager.h"

typedef struct {
	uint8_t noOfCells;																														// Number of cells in series in pack
	float batteryCapacity;																												// Battery capacity in Ah
	float cellHardUnderVoltage;																										// If the lowest cell is under this voltage -> Error situation, turn all off and power down
	float cellHardOverVoltage;																										// If the upper cell is above this voltage -> Error situation, turn all off and power down
	float cellSoftUnderVoltage;																										// If the lowest cell is under this voltage -> disable load and indicate empy battery
	float cellSoftOverVoltage;																										// If the upper cell is above this voltage -> disable charging, but keep 
	float cellBalanceDifferenceThreshold;																					// If the upper cell is more than this voltage away from the average -> start discharging this cell
	float cellBalanceStart;																												// If an upper cell is above this voltage and higher than the cellBalanceDifferenceThreshold voltage then average, start discharging 
	uint32_t cellBalanceUpdateInterval;																						// Amount of time that the balance resistor enable mask is kept
	uint8_t maxSimultaneousDischargingCells;																			// Set the maximum amount of discharging cells. This is to limit dissepated power (and thus board temperature)
	uint32_t timoutDischargeRetry;																								// If soft lower threshold limit was tripped wait this amount of time to re-enable load if cell is within threshold
	float hysteresisDischarge;																										// If the lowest cell voltage rises this amount of mV re enable output
	uint32_t timoutChargeRetry;																										// If soft higher threshold limit was tripped and cell is within acceptable limits wait this amount of time before re-enabling charge input
	float hysteresisCharge;																												// If the highest cell voltage loweres this amount of mW re enable charge input
	uint32_t timoutChargeCompleted;																								// If tricklecharging > this threshold timer declare the pack charged but keep balancing if nessesary
	uint32_t timoutChargingCompletedMinimalMismatch;															// If charger is disabled and cellvoltagemismatch is under threshold determin charged after this timout time
	float maxMismatchThreshold;
	float chargerEnabledThreshold;																								// Minimal current to stay in charge mode
	uint32_t timoutChargerDisconnected;																						// Timout for charger disconnect detection
	float minimalPrechargePercentage;																							// Output voltage threshold for precharging
	uint32_t timoutPreCharge;																											// If threshold is not reached within this time in ms goto error state
	float maxAllowedCurrent;																											// Max allowed current passing trough BMS, if limit is exceded disable output
	uint32_t displayTimoutBatteryDead;																						// Duration of displaying battery dead symbol
	uint32_t displayTimoutBatteryError;																						// Duration of displaying error symbol
	uint32_t displayTimoutSplashScreen;																						// Duration of displaying splash screen + First few samples of ADC's
	uint8_t maxUnderAndOverVoltageErrorCount;																			// Threshold that defines max amount of hard over / under voltage errors
	float notUsedCurrentThreshold;																								// Threshold that defines whether or not pack is in use.
	uint32_t notUsedTimout;																												// Delay time that defines max amount of no operation on-time. When absolute battery curren < notUsedCurrentThreshold for longer than this amount of time -> the system is disabled
	uint32_t stateOfChargeStoreInterval;																					// Interval to store state of charge information.
} modConfigGeneralConfigStructTypedef;

modConfigGeneralConfigStructTypedef* modConfigInit(void);
bool modConfigStoreAndLoadDefaultConfig(void);
bool modConfigStoreConfig(void);
bool modConfigLoadConfig(void);

#endif
