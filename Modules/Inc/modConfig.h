#ifndef __MODCONFIG_H
#define __MODCONFIG_H

#include "stdint.h"
#include "stdbool.h"

typedef struct{
	uint8_t noOfCells;																														// Number of cells in series in pack
	float cellHardUnderVoltage;																										// If the lowest cell is under this voltage -> Error situation, turn all off and power down
	float cellHardOverVoltage;																										// If the upper cell is above this voltage -> Error situation, turn all off and power down
	float cellSoftUnderVoltage;																										// If the lowest cell is under this voltage -> disable load and indicate empy battery
	float cellSoftOverVoltage;																										// If the upper cell is above this voltage -> disable charging, but keep 
	float cellBalanceDifferenceThreshold;																					// If the upper cell is more than this voltage away from the average -> start discharging this cell
	float cellBalanceStart;																												// If an upper cell is above this voltage and higher than the cellBalanceDifferenceThreshold voltage then average, start discharging 
	uint8_t maxSimultaneousDischargingCells;																			// Set the maximum amount of discharging cells. This is to limit dissepated power (and thus board temperature)
	uint32_t timoutDischargeRetry;																								// If soft lower threshold limit was tripped wait this amount of time to re-enable load if cell is within threshold
	float hysteresisDischarge;																										// If the lowest cell voltage rises this amount of mV re enable output
	uint32_t timoutChargeRetry;																										// If soft higher threshold limit was tripped and cell is within acceptable limits wait this amount of time before re-enabling charge input
	float hysteresisCharge;																												// If the highest cell voltage loweres this amount of mW re enable charge input
	uint32_t timoutChargeCompleted;																								// If tricklecharging > this threshold timer declare the pack charged but keep balancing if nessesary
	float minimalPrechargePercentage;																							// Output voltage threshold for precharging
	uint32_t timoutPreCharge;																											// If threshold is not reached within this time in ms goto error state
	float maxAllowedCurrent;																											// Max allowed current passing trough BMS, if limit is exceded disable output
} modConfigGeneralConfigStructTypedef;

void modConfigInit(modConfigGeneralConfigStructTypedef **configStructPointer);
bool modConfigStoreConfig(void);
bool modConfigLoadConfig(void);

#endif
