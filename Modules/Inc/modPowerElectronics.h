#ifndef __MODPOWERELECTRONICS_H
#define __MODPOWERELECTRONICS_H

#include "driverSWISL28022.h"
#include "driverHWADC.h"
#include "driverSWLTC6803.h"
#include "driverSWLTC6804.h"
#include "driverHWSwitches.h"
#include "driverSWEMC2305.h"
#include "modDelay.h"
#include "modConfig.h"
#include "modPowerState.h"
#include "stdbool.h"
#include "math.h"

#define NoOfCellsPossibleOnBMS	      60
#define NoOfCellMonitorsPossibleOnBMS 5
#define NoOfTempSensors               14
#define NoOfWaterSensors              6
#define PRECHARGE_PERCENTAGE 		  0.95f
#define VinErrorThreshold             10

typedef enum {
	TEMP_EXT_LTC_NTC0 = 0,									// EXT on master BMS on LTC
	TEMP_EXT_LTC_NTC1,											// EXT on master BMS on LTC
	TEMP_INT_LTC_CHIP,											// Int on master BMS inside LTC Chip
	TEMP_INT_STM_NTC,												// Int on master BMS outside STM Chip
	TEMP_EXT_ADC_NTC0,											// Ext on slave BMS OR water detect
	TEMP_EXT_ADC_NTC1,											// Ext on slave BMS OR water detect
	TEMP_EXT_ADC_NTC2,											// Ext on slave BMS OR water detect
	TEMP_EXT_ADC_NTC3,											// Ext on slave BMS OR water detect
	TEMP_EXT_ADC_NTC4,											// Ext on slave BMS OR water detect
	TEMP_EXT_ADC_NTC5,											// Ext on slave BMS OR water detect
	TEMP_INT_ADC_NTC6,											// Int on slave BMS
	TEMP_INT_ADC_NTC7,											// Int on slave BMS
	TEMP_INT_SHT,														// Int on slave BMS
	TEMP_INT_ADC_NTCAUX                     // Ext on the CAN connector
} modPowerElectronicsTemperatureSensorMapping;

typedef enum {
	PACK_STATE_ERROR_HARD_CELLVOLTAGE = 0,
	PACK_STATE_ERROR_SOFT_CELLVOLTAGE,
	PACK_STATE_ERROR_OVER_CURRENT,
	PACK_STATE_NORMAL,
} modPowerElectronicsPackOperationalCellStatesTypedef;

typedef struct {
	// Master BMS
	uint16_t throttleDutyGeneralTemperatureBMS;
	uint16_t throttleDutyChargeVoltage;
	uint16_t throttleDutyChargeTemperatureBattery;
	uint16_t throttleDutyCharge;
	uint16_t throttleDutyDischargeVoltage;
	uint16_t throttleDutyDischargeTemperatureBattery;
	uint16_t throttleDutyDischarge;
	float    SoC;
	float    SoCCapacityAh;
	OperationalStateTypedef operationalState;
	float    packVoltage;
	float    packCurrent;
	float    packPower;
	float    loCurrentLoadCurrent;
	float    loCurrentLoadVoltage;
	float    cellVoltageHigh;
	float    cellVoltageLow;
	float    cellVoltageAverage;
	float    cellVoltageMisMatch;
	uint16_t cellBalanceResistorEnableMask;
	float    temperatures[NoOfTempSensors];
	float    tempBatteryHigh;
	float    tempBatteryLow;
	float    tempBatteryAverage;
	float    tempBMSHigh;
	float    tempBMSLow;
	float    tempBMSAverage;
	uint8_t  preChargeDesired;
	uint8_t  disChargeDesired;
	uint8_t  disChargeLCAllowed;
	uint8_t  disChargeHCAllowed;
	uint8_t  chargeDesired;
	uint8_t  chargeAllowed;
	uint8_t  safetyOverCANHCSafeNSafe;
	uint8_t  chargeCurrentDetected;
	uint8_t  chargeBalanceActive;
	uint8_t  powerButtonActuated;
	uint8_t  packInSOADischarge;
  uint8_t  packInSOACharge;
	uint8_t  waterDetected;
	uint8_t  powerDownDesired;
	uint8_t  powerOnLongButtonPress;
	float    waterSensors[NoOfWaterSensors];
	uint8_t  buzzerOn;
	uint8_t  watchDogTime;
	cellMonitorCellsTypeDef cellVoltagesIndividual[NoOfCellsPossibleOnBMS];
	modPowerElectronicsPackOperationalCellStatesTypedef packOperationalCellState;
	
	// Slave BMS
	uint8_t  hiAmpShieldPresent;
	uint32_t hiCurrentLoadPreChargeDuration;
	uint8_t  hiCurrentLoadDetected;
	uint32_t hiCurrentLoadHVPreChargeDuration;
	uint8_t  hiCurrentLoadHVDetected;	
	uint8_t  hiCurrentLoadState;
	uint8_t  hiCurrentLoadStateHV;
	float    hiCurrentLoadVoltage;
	float    hiCurrentLoadCurrent;
	float		 hiCurrentLoadPower;
	float    auxVoltage;
	float    auxCurrent;
	float		 auxPower;
	uint8_t  aux0EnableDesired;
	uint8_t  aux0Enabled;
	uint8_t  aux0LoadIncorrect;
	uint8_t  aux1EnableDesired;
	uint8_t  aux1Enabled;
	uint8_t  aux1LoadIncorrect;
	uint8_t  auxDCDCEnabled;
	uint8_t  auxDCDCOutputOK;
	float    humidity;
	uint8_t  hiLoadEnabled;
	uint8_t  hiLoadPreChargeEnabled;
	uint8_t  hiLoadPreChargeError;
	uint8_t  hiLoadHVEnabled;
	uint8_t  hiLoadHVPreChargeEnabled;
	uint8_t  hiLoadHVPreChargeError;	
	uint8_t	 IOIN1;
	uint8_t  IOOUT0;
	uint8_t  FANSpeedDutyDesired;
	driverSWEMC2305FanStatusTypeDef FANStatus;
	
	// Slave sensors
	bool slaveShieldPresenceFanDriver;
	bool slaveShieldPresenceAuxADC;
	bool slaveShieldPresenceADSADC;
	bool slaveShieldPresenceMasterISL;
	bool slaveShieldPresenceMainISL;
	
	// Slave modules -> TODO move into struct.
	float    cellModuleVoltages[NoOfCellMonitorsPossibleOnBMS][12];
	uint16_t cellModuleBalanceResistorEnableMask[NoOfCellMonitorsPossibleOnBMS];
	
} modPowerElectronicsPackStateTypedef;

void  modPowerElectronicsInit(modPowerElectronicsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfig);
bool  modPowerElectronicsTask(void);
void  modPowerElectronicsAllowForcedOn(bool allowedState);
void  modPowerElectronicsSetPreCharge(bool newState);
bool  modPowerElectronicsSetDisCharge(bool newState);
void  modPowerElectronicsSetCharge(bool newState);
void  modPowerElectronicsDisableAll(void);
void  modPowerElectronicsCalculateCellStats(void);
void  modPowerElectronicsSubTaskBalaning(void);
void  modPowerElectronicsCallMinitorsCalcBalanceResistorArray(void);
void  modPowerElectronicsSubTaskVoltageWatch(void);
void  modPowerElectronicsUpdateSwitches(void);
void  modPowerElectronicsSortCells(cellMonitorCellsTypeDef *cells, uint8_t cellCount);
void  modPowerElectronicsCalcTempStats(void);
void  modPowerElectronicsCalcThrottle(void);
int32_t modPowerElectronicsMapVariableInt(int32_t inputVariable, int32_t inputLowerLimit, int32_t inputUpperLimit, int32_t outputLowerLimit, int32_t outputUpperLimit);
float modPowerElectronicsMapVariableFloat(float inputVariable, float inputLowerLimit, float inputUpperLimit, float outputLowerLimit, float outputUpperLimit);
void  modPowerElectronicsInitISL(void);
void  modPowerElectronicsCheckWaterSensors(void);
void  modPowerElectronicsSubTaskBuzzer(void);
bool  modPowerElectronicsHCSafetyCANAndPowerButtonCheck(void);
void  modPowerElectronicsResetBalanceModeActiveTimeout(void);
void  modPowerElectronicsCellMonitorsInit(void);
void  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData(void);
void  modPowerElectronicsCellMonitorsStartCellConversion(void);
void  modPowerElectronicsCellMonitorsStartLoadedCellConversion(bool PUP);
void  modPowerElectronicsCellMonitorsStartTemperatureConversion(void);
void  modPowerElectronicsCellMonitorsEnableBalanceResistors(uint16_t);
void  modPowerElectronicsCellMonitorsEnableBalanceResistorsArray(void);
void  modPowerElectronicsCellMonitorsReadVoltageFlags(uint16_t *underVoltageFlags, uint16_t *overVoltageFlags);
void  modPowerElectronicsCellMonitorsCheckAndSolveInitState(void);
void  modPowerElectronicsCellMonitorsArrayTranslate(void);
float modPowerElectronicsCalcPackCurrent(void);
void  modPowerElectronicsTerminalCellConnectionTest(int argc, const char **argv);
void  modPowerElectronicsCheckPackSOA(void);
void  modPowerElectronicsSamplePackAndLCData(void);
void  modPowerElectrinicsSamplePackVoltage(float *voltagePointer);
void  modPowerElectronicsLCSenseSample(void);
void  modPowerElectronicsLCSenseInit(void);
uint16_t modPowerElectronicsLowestInThree(uint16_t num1,uint16_t num2,uint16_t num3);

#endif
