#include "stm32f3xx_hal.h"
#include "stdbool.h"
#include "modPowerElectronics.h"
#include "modConfig.h"

void driverSWDCDCInit(modPowerElectronicsPackStateTypedef* packStateHandle, modConfigGeneralConfigStructTypedef* generalConfigHandle);
void driverSWDCDCSetEnabledState(bool newEnabledState);
bool driverSWDCDCGetEnabledState(void);
bool driverSWDCDCGetOKState(void);
void driverSWDCDCEnableTask(void);
void driverSWDCDCSetEnablePin(bool desiredEnableState);
float driverSWDCDCGetAuxVoltage(void);
float driverSWDCDCGetAuxCurrent(void);
bool driverSWDCDCCheckVoltage(float voltageActual, float voltageNominal, float accuracy);
