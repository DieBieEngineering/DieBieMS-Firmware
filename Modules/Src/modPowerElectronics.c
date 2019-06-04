#include "modPowerElectronics.h"
#include "modTerminal.h"
#include "driverIVT.h"

modPowerElectronicsPackStateTypedef *modPowerElectronicsPackStateHandle;
modConfigGeneralConfigStructTypedef *modPowerElectronicsGeneralConfigHandle;
uint32_t modPowerElectronicsMeasureIntervalLastTick;

uint32_t modPowerElectronicsChargeRetryLastTick;
uint32_t modPowerElectronicsDisChargeLCRetryLastTick;
uint32_t modPowerElectronicsDisChargeHCRetryLastTick;
uint32_t modPowerElectronicsCellBalanceUpdateLastTick;
uint32_t modPowerElectronicsTempMeasureDelayLastTick;
uint32_t modPowerElectronicsChargeCurrentDetectionLastTick;
uint32_t modPowerElectronicsBalanceModeActiveLastTick;
uint32_t modPowerElectronicsWaterDetectDelayLastTick;
uint32_t modPowerElectronicsBuzzerUpdateIntervalLastTick;
uint32_t modPowerElectronicsThrottleChargeLastTick;
uint8_t  modPowerElectronicsUnderAndOverVoltageErrorCount;
bool     modPowerElectronicsAllowForcedOnState;
uint16_t modPowerElectronicsAuxVoltageArray[3];
uint16_t tempTemperature;
uint8_t  modPowerElectronicsVinErrorCount;
configCellMonitorICTypeEnum modPowerElectronicsCellMonitorsTypeActive;
float    modPowerElectronicsChargeDiodeBypassHysteresis;
bool     modPowerElectronicsVoltageSenseError;

bool     modPowerElectronicsChargeDeratingActive;
uint32_t modPowerElectronicsChargeIncreaseLastTick;
uint32_t modPowerElectronicsSOAChargeChangeLastTick;
uint32_t modPowerElectronicsSOADisChargeChangeLastTick;
uint32_t chargeIncreaseIntervalTime;

uint16_t  calculatedChargeThrottle = 0;

uint16_t hardUnderVoltageFlags, hardOverVoltageFlags;

void modPowerElectronicsInit(modPowerElectronicsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer) {
	modPowerElectronicsGeneralConfigHandle                       = generalConfigPointer;
	modPowerElectronicsPackStateHandle                           = packState;
	modPowerElectronicsUnderAndOverVoltageErrorCount             = 0;
	modPowerElectronicsAllowForcedOnState                        = false;
	modPowerElectronicsVinErrorCount                             = 0;
	modPowerElectronicsChargeDiodeBypassHysteresis               = 0.0f;
	modPowerElectronicsVoltageSenseError                         = false;
	modPowerElectronicsChargeDeratingActive                      = false;
	
	// Init pack status
	modPowerElectronicsPackStateHandle->throttleDutyGeneralTemperatureBMS       = 0;
	modPowerElectronicsPackStateHandle->throttleDutyChargeVoltage               = 0;
	modPowerElectronicsPackStateHandle->throttleDutyChargeTemperatureBattery    = 0;
	modPowerElectronicsPackStateHandle->throttleDutyCharge                      = 0;
	modPowerElectronicsPackStateHandle->throttleDutyDischargeVoltage            = 0;
	modPowerElectronicsPackStateHandle->throttleDutyDischargeTemperatureBattery = 0;
	modPowerElectronicsPackStateHandle->throttleDutyDischarge                   = 0;
	modPowerElectronicsPackStateHandle->SoC                      = 0.0f;
	modPowerElectronicsPackStateHandle->SoCCapacityAh            = 0.0f;
	modPowerElectronicsPackStateHandle->operationalState         = OP_STATE_INIT;
	modPowerElectronicsPackStateHandle->packVoltage              = 0.0f;
	modPowerElectronicsPackStateHandle->packCurrent              = 0.0f;
	modPowerElectronicsPackStateHandle->packPower                = 0.0f;
	modPowerElectronicsPackStateHandle->loCurrentLoadCurrent     = 0.0f;
	modPowerElectronicsPackStateHandle->loCurrentLoadVoltage     = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageHigh          = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageLow           = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageAverage       = 0.0;
	modPowerElectronicsPackStateHandle->disChargeDesired         = false;
	modPowerElectronicsPackStateHandle->disChargeLCAllowed       = true;
	modPowerElectronicsPackStateHandle->disChargeHCAllowed       = true;
	modPowerElectronicsPackStateHandle->preChargeDesired         = false;
	modPowerElectronicsPackStateHandle->chargeDesired            = false;
	modPowerElectronicsPackStateHandle->chargeAllowed 					 = true;
	modPowerElectronicsPackStateHandle->safetyOverCANHCSafeNSafe = false;
	modPowerElectronicsPackStateHandle->chargeBalanceActive      = false;
	modPowerElectronicsPackStateHandle->chargeCurrentDetected    = false;
	modPowerElectronicsPackStateHandle->powerButtonActuated      = false;
	modPowerElectronicsPackStateHandle->packInSOACharge          = true;
	modPowerElectronicsPackStateHandle->packInSOADischarge       = true;	
	modPowerElectronicsPackStateHandle->watchDogTime             = 255;
	modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_NORMAL;
	modPowerElectronicsPackStateHandle->temperatures[0]          = -50.0f;
	modPowerElectronicsPackStateHandle->temperatures[1]          = -50.0f;
	modPowerElectronicsPackStateHandle->temperatures[2]          = -50.0f;
	modPowerElectronicsPackStateHandle->temperatures[3]          = -50.0f;
	modPowerElectronicsPackStateHandle->tempBatteryHigh          = 0.0f;
	modPowerElectronicsPackStateHandle->tempBatteryLow           = 0.0f;
	modPowerElectronicsPackStateHandle->tempBatteryAverage       = 0.0f;
	modPowerElectronicsPackStateHandle->tempBMSHigh              = 0.0f;
	modPowerElectronicsPackStateHandle->tempBMSLow               = 0.0f;
	modPowerElectronicsPackStateHandle->tempBMSAverage           = 0.0f;
	modPowerElectronicsPackStateHandle->waterDetected            = 0;
	modPowerElectronicsPackStateHandle->waterSensors[0]          = -50.0f;
	modPowerElectronicsPackStateHandle->waterSensors[1]          = -50.0f;
	modPowerElectronicsPackStateHandle->waterSensors[2]          = -50.0f;
	modPowerElectronicsPackStateHandle->waterSensors[3]          = -50.0f;
	modPowerElectronicsPackStateHandle->waterSensors[4]          = -50.0f;
	modPowerElectronicsPackStateHandle->waterSensors[5]          = -50.0f;
	modPowerElectronicsPackStateHandle->buzzerOn                 = false;
	modPowerElectronicsPackStateHandle->hiCurrentLoadPreChargeDuration = 0;
	modPowerElectronicsPackStateHandle->hiCurrentLoadDetected    = false;
	modPowerElectronicsPackStateHandle->hiCurrentLoadState       = 0;
	modPowerElectronicsPackStateHandle->hiCurrentLoadStateHV     = 0;
	modPowerElectronicsPackStateHandle->powerDownDesired         = false;
	modPowerElectronicsPackStateHandle->powerOnLongButtonPress   = false;
	
	// init the module variables empty
	for( uint8_t modulePointer = 0; modulePointer < NoOfCellMonitorsPossibleOnBMS; modulePointer++) {
		for(uint8_t cellPointer = 0; cellPointer < 12; cellPointer++)
			modPowerElectronicsPackStateHandle->cellModuleVoltages[modulePointer][cellPointer] = 0.0f;
		
		modPowerElectronicsPackStateHandle->cellModuleBalanceResistorEnableMask[modulePointer] = 0x0000;
	}
	
	// Init the external bus monitor
  modPowerElectronicsInitISL();
	
	// Init internal ADC
	driverHWADCInit();
	driverHWSwitchesInit();
	driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);																// Enable FET Driver
	driverHWSwitchesSetSwitchState(SWITCH_CHARGE_BYPASS,SWITCH_RESET);											// Disable charge diode bypass
	
	// Init battery stack monitor
	modPowerElectronicsCellMonitorsInit();
	
	modPowerElectronicsChargeCurrentDetectionLastTick = HAL_GetTick();
	modPowerElectronicsBalanceModeActiveLastTick      = HAL_GetTick();
	modPowerElectronicsSOAChargeChangeLastTick        = HAL_GetTick();
	modPowerElectronicsSOADisChargeChangeLastTick     = HAL_GetTick();	
	
	// Sample the first pack voltage moment
	if(modPowerElectronicsPackStateHandle->slaveShieldPresenceMasterISL)
		driverSWISL28022GetBusVoltage(ISL28022_MASTER_ADDRES,ISL28022_MASTER_BUS,&modPowerElectronicsPackStateHandle->packVoltage,0.004f);
	
	// Register terminal commands
	modTerminalRegisterCommandCallBack("testbms","Test the cell connection between cell monitor and pack and pack vs cell measurement.","[error (V)] [bal drop (mV)]",modPowerElectronicsTerminalCellConnectionTest);
};

bool modPowerElectronicsTask(void) {
	bool returnValue = false;
	
	if(modDelayTick1ms(&modPowerElectronicsMeasureIntervalLastTick,100)) {
		// reset tick for LTC Temp start conversion delay
		modPowerElectronicsTempMeasureDelayLastTick = HAL_GetTick();
		
		// Collect low current current path, pack data and check validity + recover if invalid.
		modPowerElectronicsSamplePackAndLCData();
		
		// Check whether packvoltage is whithin theoretical limits
		if(modPowerElectronicsPackStateHandle->packVoltage >= (modPowerElectronicsGeneralConfigHandle->noOfCellsSeries*modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage + 1.0f)) {
			modPowerElectronicsVoltageSenseError = true;
		}
		
		// Combine the currents based on config and calculate pack power.
		modPowerElectronicsPackStateHandle->packCurrent = modPowerElectronicsCalcPackCurrent();
		modPowerElectronicsPackStateHandle->packPower   = modPowerElectronicsPackStateHandle->packCurrent * modPowerElectronicsPackStateHandle->packVoltage;
		
		// Read the battery cell voltages and temperatures with the cell monitor ICs
		modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();
		
		// get STM32 ADC NTC temp
		driverHWADCGetNTCValue(&modPowerElectronicsPackStateHandle->temperatures[3],modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupMasterPCB],modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupMasterPCB],modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupMasterPCB],25.0f);
		
		// Calculate temperature statisticks
		modPowerElectronicsCalcTempStats();
		
		// When temperature and cellvoltages are known calculate charge and discharge throttle.
		modPowerElectronicsCalcThrottle();
		
		// Do the balancing task
		modPowerElectronicsSubTaskBalaning();
		
		// Handle buzzer desires
		modPowerElectronicsSubTaskBuzzer();
		
		// Measure cell voltages
		modPowerElectronicsCellMonitorsStartCellConversion();
		
		// Check and respond to the measured voltage values
		modPowerElectronicsSubTaskVoltageWatch();
		
		// Check water sensors and report state
		modPowerElectronicsCheckWaterSensors();
		
		// Check and respond to the measured temperature values
		modPowerElectronicsCheckPackSOA();
		
		// Check and determine whether or not there is a charge current and we need to balance.
		if(modPowerElectronicsPackStateHandle->packCurrent >= modPowerElectronicsGeneralConfigHandle->chargerEnabledThreshold) {
			if(modDelayTick1ms(&modPowerElectronicsChargeCurrentDetectionLastTick,5000)) {
				modPowerElectronicsPackStateHandle->chargeBalanceActive = modPowerElectronicsGeneralConfigHandle->allowChargingDuringDischarge;
				modPowerElectronicsPackStateHandle->chargeCurrentDetected = true;																																								// Charge current is detected after 2 seconds
			}
			
			if(modPowerElectronicsPackStateHandle->chargeCurrentDetected) {
				modPowerElectronicsResetBalanceModeActiveTimeout();
			}
		}else{
			modPowerElectronicsPackStateHandle->chargeCurrentDetected = false;
			modPowerElectronicsChargeCurrentDetectionLastTick = HAL_GetTick();
		}
		
		// Control the charge input bypass diode
		if(modPowerElectronicsPackStateHandle->chargeDesired && modPowerElectronicsPackStateHandle->chargeAllowed && (modPowerElectronicsPackStateHandle->packCurrent >= (modPowerElectronicsGeneralConfigHandle->chargerEnabledThreshold + modPowerElectronicsChargeDiodeBypassHysteresis + 0.5f))){
			driverHWSwitchesSetSwitchState(SWITCH_CHARGE_BYPASS,SWITCH_SET);
			modPowerElectronicsChargeDiodeBypassHysteresis = -0.2f;
		}else{
		  driverHWSwitchesSetSwitchState(SWITCH_CHARGE_BYPASS,SWITCH_RESET);
			modPowerElectronicsChargeDiodeBypassHysteresis = 0.2f;
		}
		
		// TODO: have balance time configureable
		if(modDelayTick1ms(&modPowerElectronicsBalanceModeActiveLastTick,10*60*1000)) {																																			// When a charge current is derected, balance for 10 minutes
			modPowerElectronicsPackStateHandle->chargeBalanceActive = false;
		}
		
		modPowerElectronicsPackStateHandle->powerButtonActuated = modPowerStateGetButtonPressedState();
		
		returnValue = true;
	}else
		returnValue = false;
	
	if(modDelayTick1msNoRST(&modPowerElectronicsTempMeasureDelayLastTick,50))
		modPowerElectronicsCellMonitorsStartTemperatureConversion();
	
	return returnValue;
};

void modPowerElectronicsAllowForcedOn(bool allowedState){
	modPowerElectronicsAllowForcedOnState = allowedState;
}

void modPowerElectronicsSetPreCharge(bool newState) {
	static bool preChargeLastState = false;
	
	if(preChargeLastState != newState) {
		preChargeLastState = newState;
		
		if(newState)
			driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);
		
		modPowerElectronicsPackStateHandle->preChargeDesired = newState;
		modPowerElectronicsUpdateSwitches();
	}
};

bool modPowerElectronicsSetDisCharge(bool newState) {
	static bool dischargeLastState = false;
	
	if(dischargeLastState != newState) {
		if(newState)
			driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET); 
		
		modPowerElectronicsPackStateHandle->disChargeDesired = newState;
		modPowerElectronicsUpdateSwitches();
		dischargeLastState = newState;
	}
	
	if((modPowerElectronicsPackStateHandle->loCurrentLoadVoltage < PRECHARGE_PERCENTAGE*(modPowerElectronicsPackStateHandle->packVoltage)) && modPowerElectronicsGeneralConfigHandle->LCUsePrecharge) // Prevent turn on with to low output voltage
		return false;																																			                                                  // Load voltage to low (output not precharged enough) return whether or not precharge is needed.
	else
		return true;
};

void modPowerElectronicsSetCharge(bool newState) {
	static bool chargeLastState = false;
	
	if(chargeLastState != newState) {
		chargeLastState = newState;
	
		if(newState)
			driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);
		
		modPowerElectronicsPackStateHandle->chargeDesired = newState;
		modPowerElectronicsUpdateSwitches();
}
};

void modPowerElectronicsDisableAll(void) {
	if(modPowerElectronicsPackStateHandle->disChargeDesired | modPowerElectronicsPackStateHandle->preChargeDesired | modPowerElectronicsPackStateHandle->chargeDesired) {
		modPowerElectronicsPackStateHandle->disChargeDesired = false;
		modPowerElectronicsPackStateHandle->preChargeDesired = false;
		modPowerElectronicsPackStateHandle->chargeDesired = false;
		driverHWSwitchesDisableAll();
	}
};

void modPowerElectronicsCalculateCellStats(void) {
	float cellVoltagesSummed = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageHigh = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageLow = 10.0f;
	
	for(uint8_t cellPointer = 0; cellPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries; cellPointer++) {
		cellVoltagesSummed += modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;
		
		if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage > modPowerElectronicsPackStateHandle->cellVoltageHigh)
			modPowerElectronicsPackStateHandle->cellVoltageHigh = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;
		
		if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage < modPowerElectronicsPackStateHandle->cellVoltageLow)
			modPowerElectronicsPackStateHandle->cellVoltageLow = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;		
	}
	
	modPowerElectronicsPackStateHandle->cellVoltageAverage = cellVoltagesSummed/modPowerElectronicsGeneralConfigHandle->noOfCellsSeries;
	modPowerElectronicsPackStateHandle->cellVoltageMisMatch = modPowerElectronicsPackStateHandle->cellVoltageHigh - modPowerElectronicsPackStateHandle->cellVoltageLow;
};

void modPowerElectronicsSubTaskBalaning(void) {
	static uint32_t delayTimeHolder = 100;
	static bool     delaytoggle = false;
	cellMonitorCellsTypeDef sortedCellArray[modPowerElectronicsGeneralConfigHandle->noOfCellsSeries];
	
	if(modDelayTick1ms(&modPowerElectronicsCellBalanceUpdateLastTick,delayTimeHolder)) {
		delaytoggle ^= true;
		delayTimeHolder = delaytoggle ? modPowerElectronicsGeneralConfigHandle->cellBalanceUpdateInterval : 200;
		
		if(delaytoggle) {
			for(int k=0; k<modPowerElectronicsGeneralConfigHandle->noOfCellsSeries; k++) {
				sortedCellArray[k] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[k];	// This will contain the voltages that are unloaded by balance resistors
			}
				
			modPowerElectronicsSortCells(sortedCellArray,modPowerElectronicsGeneralConfigHandle->noOfCellsSeries);
			
			
			//temp remove true
			if((modPowerElectronicsPackStateHandle->chargeDesired && !modPowerElectronicsPackStateHandle->disChargeDesired) || modPowerElectronicsPackStateHandle->chargeBalanceActive || true) {																							// Check if charging is desired. Removed: || !modPowerElectronicsPackStateHandle->chargeAllowed
				// Old for(uint8_t i = 0; i < modPowerElectronicsGeneralConfigHandle->maxSimultaneousDischargingCells; i++) {
				for(uint8_t i = 0; i < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries; i++) {
					if(sortedCellArray[i].cellVoltage >= (modPowerElectronicsPackStateHandle->cellVoltageLow + modPowerElectronicsGeneralConfigHandle->cellBalanceDifferenceThreshold)) {
						if(sortedCellArray[i].cellVoltage >= modPowerElectronicsGeneralConfigHandle->cellBalanceStart) {
							modPowerElectronicsPackStateHandle->cellVoltagesIndividual[sortedCellArray[i].cellNumber].cellBleedActive = true;
						}else{
						  modPowerElectronicsPackStateHandle->cellVoltagesIndividual[sortedCellArray[i].cellNumber].cellBleedActive = false;
						}
					}else{
						modPowerElectronicsPackStateHandle->cellVoltagesIndividual[sortedCellArray[i].cellNumber].cellBleedActive = false;
					}
				}
			}
		}
		
		//modPowerElectronicsPackStateHandle->cellBalanceResistorEnableMask = cellBalanceMaskEnableRegister;
		
		//if(lastCellBalanceRegister != cellBalanceMaskEnableRegister)
		//	modPowerElectronicsCellMonitorsEnableBalanceResistors(cellBalanceMaskEnableRegister);
		//lastCellBalanceRegister = cellBalanceMaskEnableRegister;
		
		modPowerElectronicsCallMinitorsCalcBalanceResistorArray();
		modPowerElectronicsCellMonitorsEnableBalanceResistorsArray();
	}
};

void modPowerElectronicsCallMinitorsCalcBalanceResistorArray(void) {
	uint8_t modulePointer = 0;
	uint8_t cellInMaskPointer = 0;
	
	// Clear array
	for(uint8_t moduleClearPointer = 0; moduleClearPointer < NoOfCellMonitorsPossibleOnBMS; moduleClearPointer++) 
		modPowerElectronicsPackStateHandle->cellModuleBalanceResistorEnableMask[moduleClearPointer] = 0;
	
	for(uint8_t cellPointer = 0; cellPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries; cellPointer++) {
		modulePointer = cellPointer/modPowerElectronicsGeneralConfigHandle->noOfCellsPerModule;
		cellInMaskPointer = cellPointer % modPowerElectronicsGeneralConfigHandle->noOfCellsPerModule;
		
		if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellBleedActive)
		  modPowerElectronicsPackStateHandle->cellModuleBalanceResistorEnableMask[modulePointer] |= (1 << cellInMaskPointer);
		else
			modPowerElectronicsPackStateHandle->cellModuleBalanceResistorEnableMask[modulePointer] &= ~(1 << cellInMaskPointer);
	}
}

// Old balance function
void modPowerElectronicsSubTaskBalaningOld(void) {
	static uint32_t delayTimeHolder = 100;
	static uint16_t lastCellBalanceRegister = 0;
	static bool     delaytoggle = false;
	uint16_t        cellBalanceMaskEnableRegister = 0;
	cellMonitorCellsTypeDef sortedCellArray[modPowerElectronicsGeneralConfigHandle->noOfCellsSeries];
	
	if(modDelayTick1ms(&modPowerElectronicsCellBalanceUpdateLastTick,delayTimeHolder)) {
		delaytoggle ^= true;
		delayTimeHolder = delaytoggle ? modPowerElectronicsGeneralConfigHandle->cellBalanceUpdateInterval : 200;
		
		if(delaytoggle) {
			for(int k=0; k<modPowerElectronicsGeneralConfigHandle->noOfCellsSeries; k++) {
				sortedCellArray[k] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[k];	// This will contain the voltages that are unloaded by balance resistors
			}
				
			modPowerElectronicsSortCells(sortedCellArray,modPowerElectronicsGeneralConfigHandle->noOfCellsSeries);
			
			if((modPowerElectronicsPackStateHandle->chargeDesired && !modPowerElectronicsPackStateHandle->disChargeDesired) || modPowerElectronicsPackStateHandle->chargeBalanceActive) {																							// Check if charging is desired. Removed: || !modPowerElectronicsPackStateHandle->chargeAllowed
				for(uint8_t i = 0; i < modPowerElectronicsGeneralConfigHandle->maxSimultaneousDischargingCells; i++) {
					if(sortedCellArray[i].cellVoltage >= (modPowerElectronicsPackStateHandle->cellVoltageLow + modPowerElectronicsGeneralConfigHandle->cellBalanceDifferenceThreshold)) {
						if(sortedCellArray[i].cellVoltage >= modPowerElectronicsGeneralConfigHandle->cellBalanceStart) {
							cellBalanceMaskEnableRegister |= (1 << sortedCellArray[i].cellNumber);
						}
					};
				}
			}
		}
		
		modPowerElectronicsPackStateHandle->cellBalanceResistorEnableMask = cellBalanceMaskEnableRegister;
		
		if(lastCellBalanceRegister != cellBalanceMaskEnableRegister)
			modPowerElectronicsCellMonitorsEnableBalanceResistors(cellBalanceMaskEnableRegister);
		lastCellBalanceRegister = cellBalanceMaskEnableRegister;
		
		modPowerElectronicsCellMonitorsEnableBalanceResistorsArray();
	}
};

void modPowerElectronicsSubTaskVoltageWatch(void) {
	static bool lastdisChargeLCAllowed = false;
	static bool lastChargeAllowed = false;
	//uint16_t hardUnderVoltageFlags, hardOverVoltageFlags;
	
	modPowerElectronicsCellMonitorsReadVoltageFlags(&hardUnderVoltageFlags,&hardOverVoltageFlags);
	modPowerElectronicsCalculateCellStats();
	
	if(modPowerElectronicsPackStateHandle->packOperationalCellState != PACK_STATE_ERROR_HARD_CELLVOLTAGE) {
		// Handle soft cell voltage limits
		// Low current
		if(modPowerElectronicsPackStateHandle->cellVoltageLow <= modPowerElectronicsGeneralConfigHandle->cellLCSoftUnderVoltage) {
			modPowerElectronicsPackStateHandle->disChargeLCAllowed = false;
			modPowerElectronicsDisChargeLCRetryLastTick = HAL_GetTick();
		}
		
		// High current
		if(modPowerElectronicsPackStateHandle->cellVoltageLow <= modPowerElectronicsGeneralConfigHandle->cellHCSoftUnderVoltage) {
			modPowerElectronicsPackStateHandle->disChargeHCAllowed = false;
			modPowerElectronicsDisChargeHCRetryLastTick = HAL_GetTick();
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageHigh >= modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage) {
			modPowerElectronicsPackStateHandle->chargeAllowed = false;
			modPowerElectronicsChargeRetryLastTick = HAL_GetTick();
		}
		
		// Low current
		if(modPowerElectronicsPackStateHandle->cellVoltageLow >= (modPowerElectronicsGeneralConfigHandle->cellLCSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->hysteresisDischarge)) {
			if(modDelayTick1ms(&modPowerElectronicsDisChargeLCRetryLastTick,modPowerElectronicsGeneralConfigHandle->timeoutDischargeRetry))
				modPowerElectronicsPackStateHandle->disChargeLCAllowed = true;
		}
		
		// High current
		if(modPowerElectronicsPackStateHandle->cellVoltageLow >= (modPowerElectronicsGeneralConfigHandle->cellHCSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->hysteresisDischarge)) {
			if(modDelayTick1ms(&modPowerElectronicsDisChargeHCRetryLastTick,modPowerElectronicsGeneralConfigHandle->timeoutDischargeRetry))
				modPowerElectronicsPackStateHandle->disChargeHCAllowed = true;
		}		
		
		if(modPowerElectronicsPackStateHandle->cellVoltageHigh <= (modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->hysteresisCharge)) {
			if(modDelayTick1ms(&modPowerElectronicsChargeRetryLastTick,modPowerElectronicsGeneralConfigHandle->timeoutChargeRetry))
				modPowerElectronicsPackStateHandle->chargeAllowed = true;
		}
		
		if(modPowerElectronicsPackStateHandle->chargeAllowed && modPowerElectronicsPackStateHandle->disChargeLCAllowed)
			modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_NORMAL;
		else
			modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_ERROR_SOFT_CELLVOLTAGE;
	}
	
	// Handle hard cell voltage limits
	if(modPowerElectronicsVoltageSenseError || hardUnderVoltageFlags || hardOverVoltageFlags || (modPowerElectronicsPackStateHandle->packVoltage > modPowerElectronicsGeneralConfigHandle->noOfCellsSeries*modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage)) {
		if(modPowerElectronicsUnderAndOverVoltageErrorCount++ > modPowerElectronicsGeneralConfigHandle->maxUnderAndOverVoltageErrorCount)
			modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_ERROR_HARD_CELLVOLTAGE;
		modPowerElectronicsPackStateHandle->disChargeLCAllowed = false;
		modPowerElectronicsPackStateHandle->chargeAllowed = false;
	}else
		modPowerElectronicsUnderAndOverVoltageErrorCount = 0;
	
	
	// update outputs directly if needed
	if((lastChargeAllowed != modPowerElectronicsPackStateHandle->chargeAllowed) || (lastdisChargeLCAllowed != modPowerElectronicsPackStateHandle->disChargeLCAllowed)) {
		lastChargeAllowed = modPowerElectronicsPackStateHandle->chargeAllowed;
		lastdisChargeLCAllowed = modPowerElectronicsPackStateHandle->disChargeLCAllowed;
		modPowerElectronicsUpdateSwitches();
	}
};

// Update switch states, should be called after every desired/allowed switch state change
void modPowerElectronicsUpdateSwitches(void) {
	// Do the actual power switching in here
	
	//Handle pre charge output
	if(modPowerElectronicsPackStateHandle->preChargeDesired && (modPowerElectronicsPackStateHandle->disChargeLCAllowed || modPowerElectronicsAllowForcedOnState))
		driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE,(driverHWSwitchesStateTypedef)SWITCH_SET);
	else
		driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE,(driverHWSwitchesStateTypedef)SWITCH_RESET);
	
	//Handle discharge output
	if(modPowerElectronicsPackStateHandle->disChargeDesired && (modPowerElectronicsPackStateHandle->disChargeLCAllowed || modPowerElectronicsAllowForcedOnState))
		driverHWSwitchesSetSwitchState(SWITCH_DISCHARGE,(driverHWSwitchesStateTypedef)SWITCH_SET);
	else
		driverHWSwitchesSetSwitchState(SWITCH_DISCHARGE,(driverHWSwitchesStateTypedef)SWITCH_RESET);
	
	//Handle charge input
	if(modPowerElectronicsPackStateHandle->chargeDesired && modPowerElectronicsPackStateHandle->chargeAllowed)
		driverHWSwitchesSetSwitchState(SWITCH_CHARGE,(driverHWSwitchesStateTypedef)SWITCH_SET);
	else
		driverHWSwitchesSetSwitchState(SWITCH_CHARGE,(driverHWSwitchesStateTypedef)SWITCH_RESET);
};

void modPowerElectronicsSortCells(cellMonitorCellsTypeDef *cells, uint8_t cellCount) {
	int i,j;
	cellMonitorCellsTypeDef value;

	for(i=0 ; i<(cellCount-1) ; i++) {
		for(j=0 ; j<(cellCount-i-1) ; j++) {
				if(cells[j].cellVoltage < cells[j+1].cellVoltage) {
						value = cells[j+1];
						cells[j+1] = cells[j];
						cells[j] = value;
				}
		}
	}
};

void modPowerElectronicsCalcTempStats(void) {
	uint8_t sensorPointer;
	
	// Battery
	float   tempBatteryMax;
	float   tempBatteryMin;
	float   tempBatterySum = 0.0f;
	uint8_t tempBatterySumCount = 0;
	
	// BMS
	float   tempBMSMax;
	float   tempBMSMin;
	float   tempBMSSum = 0.0f;
	uint8_t tempBMSSumCount = 0;
	
	if(modPowerElectronicsGeneralConfigHandle->tempEnableMaskBattery){
		tempBatteryMax = -100.0f;
		tempBatteryMin = 100.0f;
	}else{
		tempBatteryMax = 0.0f;
		tempBatteryMin = 0.0f;
	}
	
	if(modPowerElectronicsGeneralConfigHandle->tempEnableMaskBMS){
		tempBMSMax = -100.0f;
		tempBMSMin = 100.0f;
	}else{
		tempBMSMax = 0.0f;
		tempBMSMin = 0.0f;
	}
	
	for(sensorPointer = 0; sensorPointer < 16; sensorPointer++){
		// Battery temperatures
		if(modPowerElectronicsGeneralConfigHandle->tempEnableMaskBattery & (1 << sensorPointer)){
			if(modPowerElectronicsPackStateHandle->temperatures[sensorPointer] > tempBatteryMax)
				tempBatteryMax = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
			
			if(modPowerElectronicsPackStateHandle->temperatures[sensorPointer] < tempBatteryMin)
				tempBatteryMin = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
			
			tempBatterySum += modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
			tempBatterySumCount++;
		}
	
		// BMS temperatures
		if(modPowerElectronicsGeneralConfigHandle->tempEnableMaskBMS & (1 << sensorPointer)){
			if(modPowerElectronicsPackStateHandle->temperatures[sensorPointer] > tempBMSMax)
				tempBMSMax = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
			
			if(modPowerElectronicsPackStateHandle->temperatures[sensorPointer] < tempBMSMin)
				tempBMSMin = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
			
			tempBMSSum += modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
			tempBMSSumCount++;
		}
	}
	
	// Battery temperatures
	modPowerElectronicsPackStateHandle->tempBatteryHigh    = tempBatteryMax;
	modPowerElectronicsPackStateHandle->tempBatteryLow     = tempBatteryMin;
	if(tempBatterySumCount)
		modPowerElectronicsPackStateHandle->tempBatteryAverage = tempBatterySum/tempBatterySumCount;
	else
		modPowerElectronicsPackStateHandle->tempBatteryAverage = 0.0f;
	
	// BMS temperatures
	modPowerElectronicsPackStateHandle->tempBMSHigh        = tempBMSMax;
	modPowerElectronicsPackStateHandle->tempBMSLow         = tempBMSMin;
	if(tempBMSSumCount)
		modPowerElectronicsPackStateHandle->tempBMSAverage = tempBMSSum/tempBMSSumCount;
	else
		modPowerElectronicsPackStateHandle->tempBMSAverage = 0.0f;
};

void modPowerElectronicsCalcThrottle(void) {
	static uint16_t filteredChargeThrottle = 0;
	static uint16_t filteredDisChargeThrottle = 0;
	
	// TODO make config to either do the throttling on the high or low current output
	// TODO Make lower percentages configurable
	
	//uint16_t  calculatedChargeThrottle = 0;
	uint16_t  calculatedDisChargeThrottle = 0;
	uint32_t  chargeIncreaseIntervalTime;
	uint16_t  chargeIncreaseRate;
	float     cellSoftUnderVoltage = modPowerElectronicsGeneralConfigHandle->cellHCSoftUnderVoltage;
	
	// Throttle charge
	float inputLowerLimitChargeVoltage               = modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->cellThrottleUpperMargin - modPowerElectronicsGeneralConfigHandle->cellThrottleUpperStart;
	float inputUpperLimitChargeVoltage               = modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->cellThrottleUpperMargin;
	float inputLowerLimitChargeTemperatureBattery    = modPowerElectronicsGeneralConfigHandle->allowedTempBattChargingMax - 3.0f;
	float inputUpperLimitChargeTemperatureBattery    = modPowerElectronicsGeneralConfigHandle->allowedTempBattChargingMax;	
	float outputLowerLimitCharge = 1000.0f;
	float outputUpperLimitCharge = 100.0f;
	
	// Throttle discharge
	float inputLowerLimitDisChargeVoltage            = cellSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->cellThrottleLowerMargin;
  float inputUpperLimitDisChargeVoltage            = cellSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->cellThrottleLowerMargin + modPowerElectronicsGeneralConfigHandle->cellThrottleLowerStart;
	float inputLowerLimitDisChargeTemperatureBattery = modPowerElectronicsGeneralConfigHandle->allowedTempBattDischargingMax - 3.0f;
	float inputUpperLimitDisChargeTemperatureBattery = modPowerElectronicsGeneralConfigHandle->allowedTempBattDischargingMax;
  float outputLowerLimitDisCharge = 50.0f;
	float outputUpperLimitDisCharge = 1000.0f;
	
	// Throttle general
	float inputLowerLimitTemperatureBMS = modPowerElectronicsGeneralConfigHandle->allowedTempBMSMax - 4.0f;
	float inputUpperLimitTemperatureBMS = modPowerElectronicsGeneralConfigHandle->allowedTempBMSMax;
	
	modPowerElectronicsPackStateHandle->throttleDutyGeneralTemperatureBMS       = (uint16_t)modPowerElectronicsMapVariableFloat(modPowerElectronicsPackStateHandle->tempBMSHigh,inputLowerLimitTemperatureBMS,inputUpperLimitTemperatureBMS,outputUpperLimitDisCharge,outputLowerLimitDisCharge);
	modPowerElectronicsPackStateHandle->throttleDutyChargeVoltage               = (uint16_t)modPowerElectronicsMapVariableFloat(modPowerElectronicsPackStateHandle->cellVoltageHigh,inputLowerLimitChargeVoltage,inputUpperLimitChargeVoltage,outputLowerLimitCharge,outputUpperLimitCharge);
	modPowerElectronicsPackStateHandle->throttleDutyChargeTemperatureBattery    = (uint16_t)modPowerElectronicsMapVariableFloat(modPowerElectronicsPackStateHandle->tempBatteryHigh,inputLowerLimitChargeTemperatureBattery,inputUpperLimitChargeTemperatureBattery,outputLowerLimitCharge,outputUpperLimitCharge);
	modPowerElectronicsPackStateHandle->throttleDutyDischargeVoltage            = (uint16_t)modPowerElectronicsMapVariableFloat(modPowerElectronicsPackStateHandle->cellVoltageLow,inputLowerLimitDisChargeVoltage,inputUpperLimitDisChargeVoltage,outputLowerLimitDisCharge,outputUpperLimitDisCharge);
	modPowerElectronicsPackStateHandle->throttleDutyDischargeTemperatureBattery = (uint16_t)modPowerElectronicsMapVariableFloat(modPowerElectronicsPackStateHandle->tempBatteryHigh,inputLowerLimitDisChargeTemperatureBattery,inputUpperLimitDisChargeTemperatureBattery,outputUpperLimitDisCharge,outputLowerLimitDisCharge);
		
	// Calculate (dis)charge throttle and pass it if in SOA
	if(modPowerElectronicsPackStateHandle->packInSOACharge){
		//modPowerElectronicsPackStateHandle->throttleDutyGeneralTemperatureBMS
		//modPowerElectronicsPackStateHandle->throttleDutyChargeVoltage
		//modPowerElectronicsPackStateHandle->throttleDutyChargeTemperatureBattery
		calculatedChargeThrottle = modPowerElectronicsLowestInThree(modPowerElectronicsPackStateHandle->throttleDutyGeneralTemperatureBMS,modPowerElectronicsPackStateHandle->throttleDutyChargeVoltage,modPowerElectronicsPackStateHandle->throttleDutyChargeTemperatureBattery);
	}else{
		calculatedChargeThrottle = 0;
	}
	
	if(modPowerElectronicsPackStateHandle->packInSOADischarge){
		//modPowerElectronicsPackStateHandle->throttleDutyGeneralTemperatureBMS
		//modPowerElectronicsPackStateHandle->throttleDutyDischargeVoltage
		//modPowerElectronicsPackStateHandle->throttleDutyDischargeTemperatureBattery
		
		calculatedDisChargeThrottle = modPowerElectronicsLowestInThree(modPowerElectronicsPackStateHandle->throttleDutyGeneralTemperatureBMS,modPowerElectronicsPackStateHandle->throttleDutyDischargeVoltage,modPowerElectronicsPackStateHandle->throttleDutyDischargeTemperatureBattery);
	}else{
		calculatedDisChargeThrottle = 0;
	}
	
	// Filter the calculated throttle charging
	if(calculatedChargeThrottle >= filteredChargeThrottle) {
		if(modPowerElectronicsChargeDeratingActive) {
			chargeIncreaseIntervalTime = 5000;
			chargeIncreaseRate         = 1;
    }else{
		  chargeIncreaseIntervalTime = 100;
			chargeIncreaseRate         = modPowerElectronicsGeneralConfigHandle->throttleChargeIncreaseRate;
		}
		
		if(modDelayTick1ms(&modPowerElectronicsChargeIncreaseLastTick,chargeIncreaseIntervalTime)){
			if(abs(calculatedChargeThrottle-filteredChargeThrottle) > chargeIncreaseRate) {
				filteredChargeThrottle += chargeIncreaseRate;		
			}else{
				filteredChargeThrottle = calculatedChargeThrottle;
			}
		}
	}else{
		modPowerElectronicsChargeDeratingActive = true;
		filteredChargeThrottle = calculatedChargeThrottle;
	}
	
	// Filter the calculated throttle discharging
	if(calculatedDisChargeThrottle >= filteredDisChargeThrottle){
		if((calculatedDisChargeThrottle-filteredDisChargeThrottle) > modPowerElectronicsGeneralConfigHandle->throttleDisChargeIncreaseRate) {
			filteredDisChargeThrottle += modPowerElectronicsGeneralConfigHandle->throttleDisChargeIncreaseRate;
		}else{
			filteredDisChargeThrottle = calculatedDisChargeThrottle;
		}
	}else{
		filteredDisChargeThrottle = calculatedDisChargeThrottle;
	}
	
  // Output the filtered output
	if(modPowerElectronicsPackStateHandle->chargeAllowed)
		modPowerElectronicsPackStateHandle->throttleDutyCharge = filteredChargeThrottle;
	else 
		modPowerElectronicsPackStateHandle->throttleDutyCharge = 0;
	
	// TODO have it configurable to either HC or LC
	if(modPowerElectronicsPackStateHandle->disChargeHCAllowed)
		modPowerElectronicsPackStateHandle->throttleDutyDischarge = filteredDisChargeThrottle;
	else 
		modPowerElectronicsPackStateHandle->throttleDutyDischarge = 0;
}

int32_t modPowerElectronicsMapVariableInt(int32_t inputVariable, int32_t inputLowerLimit, int32_t inputUpperLimit, int32_t outputLowerLimit, int32_t outputUpperLimit) {
	inputVariable = inputVariable < inputLowerLimit ? inputLowerLimit : inputVariable;
	inputVariable = inputVariable > inputUpperLimit ? inputUpperLimit : inputVariable;
	
	return (inputVariable - inputLowerLimit) * (outputUpperLimit - outputLowerLimit) / (inputUpperLimit - inputLowerLimit) + outputLowerLimit;
}

float modPowerElectronicsMapVariableFloat(float inputVariable, float inputLowerLimit, float inputUpperLimit, float outputLowerLimit, float outputUpperLimit) {
	inputVariable = inputVariable < inputLowerLimit ? inputLowerLimit : inputVariable;
	inputVariable = inputVariable > inputUpperLimit ? inputUpperLimit : inputVariable;
	
	return (inputVariable - inputLowerLimit) * (outputUpperLimit - outputLowerLimit) / (inputUpperLimit - inputLowerLimit) + outputLowerLimit;
}

void modPowerElectronicsInitISL(void) {
	// Init BUS monitor
	driverSWISL28022InitStruct ISLInitStruct;
	ISLInitStruct.ADCSetting = ADC_128_64010US;
	ISLInitStruct.busVoltageRange = BRNG_60V_1;
	ISLInitStruct.currentShuntGain = PGA_4_160MV;
	ISLInitStruct.Mode = MODE_SHUNTANDBUS_CONTINIOUS;
	driverSWISL28022Init(ISL28022_MASTER_ADDRES,ISL28022_MASTER_BUS,ISLInitStruct);
}

void modPowerElectronicsCheckPackSOA(void) {
	static float hysteresysBMS       = -2.0f;
	static float hysteresysDischarge = -2.0f;
	static float hysteresysCharge    = -2.0f;
	
	static bool  lastPackInSOACharge    = true;
	static bool  lastPackInSOADisCharge = true;
	
	bool outsideLimitsBMS       = false;
	bool outsideLimitsDischarge = false;
	bool outsideLimitsCharge    = false;	
	
	outsideLimitsBMS |= (modPowerElectronicsVinErrorCount >= VinErrorThreshold) ? true : false;
	
	// Check BMS Limits
	if(modPowerElectronicsGeneralConfigHandle->tempEnableMaskBMS) {
		outsideLimitsBMS       |= (modPowerElectronicsPackStateHandle->tempBMSHigh     > (modPowerElectronicsGeneralConfigHandle->allowedTempBMSMax + hysteresysBMS) ) ? true : false;
		outsideLimitsBMS       |= (modPowerElectronicsPackStateHandle->tempBMSLow      < (modPowerElectronicsGeneralConfigHandle->allowedTempBMSMin - hysteresysBMS) ) ? true : false;
		
    if(outsideLimitsBMS)
			hysteresysBMS = -2.0f;
		else
			hysteresysBMS = 2.0f;
	}
	
	// Check Battery Limits discharge
	if(modPowerElectronicsGeneralConfigHandle->tempEnableMaskBattery) {
		outsideLimitsDischarge |= (modPowerElectronicsPackStateHandle->tempBatteryHigh > (modPowerElectronicsGeneralConfigHandle->allowedTempBattDischargingMax + hysteresysDischarge) ) ? true : false;
		outsideLimitsDischarge |= (modPowerElectronicsPackStateHandle->tempBatteryLow  < (modPowerElectronicsGeneralConfigHandle->allowedTempBattDischargingMin - hysteresysDischarge) ) ? true : false;
		
    if(outsideLimitsDischarge)
			hysteresysDischarge = -2.0f;
		else
			hysteresysDischarge = 2.0f;
	}
	
	// Check Battery Limits charge
	if(modPowerElectronicsGeneralConfigHandle->tempEnableMaskBattery) {
		outsideLimitsCharge    |= (modPowerElectronicsPackStateHandle->tempBatteryHigh > (modPowerElectronicsGeneralConfigHandle->allowedTempBattChargingMax + hysteresysCharge) ) ? true : false;
		outsideLimitsCharge    |= (modPowerElectronicsPackStateHandle->tempBatteryLow  < (modPowerElectronicsGeneralConfigHandle->allowedTempBattChargingMin - hysteresysCharge) ) ? true : false;

    if(outsideLimitsCharge)
			hysteresysCharge = -2.0f;
		else
			hysteresysCharge = 2.0f;
	}
	
	// DisCharge delayed response
	if(lastPackInSOADisCharge != !(outsideLimitsBMS || outsideLimitsDischarge)){
		if(modDelayTick1ms(&modPowerElectronicsSOADisChargeChangeLastTick,1000)) {
			lastPackInSOADisCharge = modPowerElectronicsPackStateHandle->packInSOADischarge = !(outsideLimitsBMS || outsideLimitsDischarge);
		}
	}else{
		modPowerElectronicsSOADisChargeChangeLastTick = HAL_GetTick();
	}	
	
	// Charge delayed response
	if(lastPackInSOACharge != !(outsideLimitsBMS || outsideLimitsCharge)){
		if(modDelayTick1ms(&modPowerElectronicsSOAChargeChangeLastTick,1000)) {
			lastPackInSOACharge = modPowerElectronicsPackStateHandle->packInSOACharge = !(outsideLimitsBMS || outsideLimitsCharge);
		}
	}else{
		modPowerElectronicsSOAChargeChangeLastTick = HAL_GetTick();
	}
}

void modPowerElectronicsCheckWaterSensors(void) {
  uint8_t sensorPointer;
	uint8_t sensorCount = 0;
	bool    waterDetected = false;
	
	// extract sensor values from sensor struct
	for(sensorPointer = 0; sensorPointer < 16; sensorPointer++){
		if((modPowerElectronicsGeneralConfigHandle->waterSensorEnableMask & (1<<sensorPointer)) && (sensorCount < 6)) {
		  modPowerElectronicsPackStateHandle->waterSensors[sensorCount] = modPowerElectronicsPackStateHandle->temperatures[sensorPointer];
			
			if(modPowerElectronicsPackStateHandle->waterSensors[sensorCount] >= modPowerElectronicsGeneralConfigHandle->waterSensorThreshold)
				waterDetected = true;
			
			sensorCount++;
		}
	}
	
	// Report water detected delayed
	if(waterDetected){
	  if(modDelayTick1ms(&modPowerElectronicsWaterDetectDelayLastTick,1000)){
			modPowerElectronicsPackStateHandle->waterDetected = true;
		}
	}else{
		modPowerElectronicsWaterDetectDelayLastTick = HAL_GetTick();
		
		if(!modPowerElectronicsGeneralConfigHandle->buzzerSingalPersistant) {
		  modPowerElectronicsPackStateHandle->waterDetected = false;
		}
	}
}

void modPowerElectronicsSubTaskBuzzer(void) {
  bool buzzerEnabledState = false;
	
	// determin whether buzzer should sound
  switch(modPowerElectronicsGeneralConfigHandle->buzzerSignalSource) {
		case buzzerSourceOff:
			buzzerEnabledState = false;
			break;
		case buzzerSourceOn:
			buzzerEnabledState = true;
			break;
		case buzzerSourceAll:
			buzzerEnabledState |= modPowerElectronicsPackStateHandle->waterDetected;
			break;
		case buzzerSourceWater:
			buzzerEnabledState = modPowerElectronicsPackStateHandle->waterDetected;
			break;
		case buzzerSourceHC:
			break;
		case buzzerSourceLC:
			break;
		case buzzerSourceSOA:
			break;
		default:
			buzzerEnabledState = false;
			break;
	}
	
  // update buzzer state every second
	if(modDelayTick1ms(&modPowerElectronicsBuzzerUpdateIntervalLastTick,1000)) {
		if(buzzerEnabledState) {
		  modEffectChangeState(STAT_BUZZER,(STATStateTypedef)modPowerElectronicsGeneralConfigHandle->buzzerSignalType);
			modPowerElectronicsPackStateHandle->buzzerOn = true;
		}else{
		  if(!modPowerElectronicsGeneralConfigHandle->buzzerSingalPersistant) {
				modEffectChangeState(STAT_BUZZER,STAT_RESET);
				modPowerElectronicsPackStateHandle->buzzerOn = false;
			}
		}
	}
}

bool modPowerElectronicsHCSafetyCANAndPowerButtonCheck(void) {
	if(modPowerElectronicsGeneralConfigHandle->useCANSafetyInput)
		return (modPowerElectronicsPackStateHandle->safetyOverCANHCSafeNSafe && (modPowerElectronicsPackStateHandle->powerButtonActuated | modPowerElectronicsGeneralConfigHandle->pulseToggleButton));
	else
		return true;
}

void modPowerElectronicsResetBalanceModeActiveTimeout(void) {
	modPowerElectronicsBalanceModeActiveLastTick = HAL_GetTick();
}

void modPowerElectronicsCellMonitorsInit(void){
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			driverLTC6803ConfigStructTypedef configStruct;
			configStruct.WatchDogFlag             = false;																									// Don't change watchdog
			configStruct.GPIO1                    = false;
			configStruct.GPIO2                    = true;
			configStruct.LevelPolling             = true;																										// This wil make the LTC SDO high (and low when adc is busy) instead of toggling when polling for ADC ready and AD conversion finished.
			configStruct.CDCMode                  = 2;																											// Comperator period = 13ms, Vres powerdown = no.
			configStruct.DisChargeEnableMask      = 0x0000;																							    // Disable all discharge resistors
			configStruct.noOfCells                = modPowerElectronicsGeneralConfigHandle->noOfCellsSeries;// Number of cells that can cause interrupt
			configStruct.CellVoltageConversionMode = LTC6803StartCellVoltageADCConversionAll;				        // Use normal cell conversion mode, in the future -> check for lose wires on initial startup.
			configStruct.CellUnderVoltageLimit    = modPowerElectronicsGeneralConfigHandle->cellHardUnderVoltage;// Set under limit to XV	-> This should cause error state
			configStruct.CellOverVoltageLimit     = modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage;// Set upper limit to X.XXV  -> This should cause error state
			driverSWLTC6803Init(configStruct,modPowerElectronicsGeneralConfigHandle->cellMonitorICCount);   // Config the LTC6803 and start measuring
			
			// Safety signal is managed by LTC, the controller should release it: it is configured as open drain.
			driverHWSwitchesSetSwitchState(SWITCH_SAFETY_OUTPUT,SWITCH_SET);
			
		}break;
		case CELL_MON_LTC6804_1: {
			driverLTC6804ConfigStructTypedef configStruct;
			configStruct.GPIO1                    = true;																										// Do not pull down this pin (false = pull down)
			configStruct.GPIO2                    = true;
#ifdef TDHVSolar
			configStruct.GPIO3                    = false;// BMS interlock
#else
			configStruct.GPIO3                    = true;// BMS interlock
#endif
			configStruct.GPIO4                    = true;																										// 
			configStruct.GPIO5                    = true;																										//
			configStruct.ReferenceON              = true;																										// Reference ON
			configStruct.ADCOption                = true;																									  // ADC Option register for configuration of over sampling ratio
			configStruct.noOfCells                = modPowerElectronicsGeneralConfigHandle->noOfCellsPerModule;// Number of cells to monitor (that can cause interrupt)
			configStruct.DisChargeEnableMask      = 0x0000;																									// Set enable state of discharge, 1=EnableDischarge, 0=DisableDischarge
			configStruct.DischargeTimout          = 0;																											// Discharge timout value / limit
			configStruct.CellUnderVoltageLimit    = modPowerElectronicsGeneralConfigHandle->cellHardUnderVoltage; // Undervoltage level, cell voltages under this limit will cause interrupt
			configStruct.CellOverVoltageLimit     = modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage;  // Over voltage limit, cell voltages over this limit will cause interrupt
			driverSWLTC6804Init(configStruct,modPowerElectronicsGeneralConfigHandle->cellMonitorICCount);   // Config the LTC6804
			
			// Safety signal is managed by the controller, it is configured as open drain and will be kept low by. watchdog will make the output to be released.
			driverHWSwitchesSetSwitchState(SWITCH_SAFETY_OUTPUT,SWITCH_RESET);
		}break;
		default:
			break;
	}
	
	modPowerElectronicsCellMonitorsTypeActive = (configCellMonitorICTypeEnum)modPowerElectronicsGeneralConfigHandle->cellMonitorType;
}

void modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData(void){
	modPowerElectronicsCellMonitorsCheckAndSolveInitState();
	
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			// Check if LTC is still running and if so read cell voltages, if not reinit.
			driverLTC6803ConfigStructTypedef modPowerElectronicsLTCconfigStruct;
			driverSWLTC6803ReadConfig(&modPowerElectronicsLTCconfigStruct);
			if(!modPowerElectronicsLTCconfigStruct.CDCMode)
				driverSWLTC6803ReInit();																														// Something went wrong, reinit the battery stack monitor.
			else
				driverSWLTC6803ReadCellVoltages(modPowerElectronicsPackStateHandle->cellVoltagesIndividual);

			// Check if LTC has discharge resistor enabled while not charging
			if(!modPowerElectronicsPackStateHandle->chargeDesired && modPowerElectronicsLTCconfigStruct.DisChargeEnableMask)
				driverSWLTC6803ReInit();																														// Something went wrong, reinit the battery stack monitor.

			// Collect LTC temperature data
			driverSWLTC6803ReadTempVoltages(modPowerElectronicsAuxVoltageArray);
			modPowerElectronicsPackStateHandle->temperatures[0] = driverSWLTC6803ConvertTemperatureExt(modPowerElectronicsAuxVoltageArray[0],modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupLTCExt],25.0f);
			modPowerElectronicsPackStateHandle->temperatures[1] = driverSWLTC6803ConvertTemperatureExt(modPowerElectronicsAuxVoltageArray[1],modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupLTCExt],25.0f);
			modPowerElectronicsPackStateHandle->temperatures[2] = driverSWLTC6803ConvertTemperatureInt(modPowerElectronicsAuxVoltageArray[2]);
		}break;
		case CELL_MON_LTC6804_1: {
			// Check config valid and reinit
			// TODO: Implement
			
			// Read cell voltages			
			driverSWLTC6804ReadCellVoltagesArray(modPowerElectronicsPackStateHandle->cellModuleVoltages);
			modPowerElectronicsCellMonitorsArrayTranslate();
			// Convert modules to full array
			
			// Read aux voltages
			//TODO these were cometnted
			driverSWLTC6804ReadAuxSensors(modPowerElectronicsAuxVoltageArray);
			modPowerElectronicsPackStateHandle->temperatures[0] =	modPowerElectronicsPackStateHandle->temperatures[1] = driverSWLTC6804ConvertTemperatureExt(modPowerElectronicsAuxVoltageArray[1],modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupLTCExt],25.0f);
		}break;
		default:
			break;
	}
}

void modPowerElectronicsCellMonitorsArrayTranslate(void) {
	uint8_t individualCellPointer = 0;
	
  for(uint8_t modulePointer = 0; modulePointer < modPowerElectronicsGeneralConfigHandle->cellMonitorICCount; modulePointer++) {
	  for(uint8_t modulePointerCell = 0; modulePointerCell < modPowerElectronicsGeneralConfigHandle->noOfCellsPerModule; modulePointerCell++) {
			modPowerElectronicsPackStateHandle->cellVoltagesIndividual[individualCellPointer].cellVoltage = modPowerElectronicsPackStateHandle->cellModuleVoltages[modulePointer][modulePointerCell];
			modPowerElectronicsPackStateHandle->cellVoltagesIndividual[individualCellPointer].cellNumber = individualCellPointer++;
		}
	}
}

void modPowerElectronicsCellMonitorsStartCellConversion(void) {
	modPowerElectronicsCellMonitorsCheckAndSolveInitState();
	
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			driverSWLTC6803StartCellVoltageConversion();
			driverSWLTC6803ResetCellVoltageRegisters();
		}break;
		case CELL_MON_LTC6804_1: {
			driverSWLTC6804StartCellAndAuxVoltageConversion(MD_FILTERED,DCP_ENABLED);
			driverSWLTC6804ResetCellVoltageRegisters();
		}break;
		default:
			break;
	}
}

void modPowerElectronicsCellMonitorsStartLoadedCellConversion(bool PUP) {
	modPowerElectronicsCellMonitorsCheckAndSolveInitState();
	
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			if(PUP)
			  driverSWLTC6803StartCellVoltageConversion();
			else
				driverSWLTC6803StartLoadedCellVoltageConversion();
				
			driverSWLTC6803ResetCellVoltageRegisters();
		}break;
		case CELL_MON_LTC6804_1: {
		  driverSWLTC6804StartLoadedCellVoltageConversion(MD_FILTERED,DCP_ENABLED,CELL_CH_ALL,PUP);
			driverSWLTC6804ResetCellVoltageRegisters();
		}break;
		default:
			break;
	}
}

void modPowerElectronicsCellMonitorsStartTemperatureConversion(void) {
	modPowerElectronicsCellMonitorsCheckAndSolveInitState();
	
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			driverSWLTC6803StartTemperatureVoltageConversion();
		}break;
		case CELL_MON_LTC6804_1: {
      // Measured simultaniously with cell voltages
		}break;
		default:
			break;
	}
}

// Make legacy
void modPowerElectronicsCellMonitorsEnableBalanceResistors(uint16_t balanceEnableMask){
	modPowerElectronicsCellMonitorsCheckAndSolveInitState();
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			driverSWLTC6803EnableBalanceResistors(balanceEnableMask);
		}break;
		case CELL_MON_LTC6804_1: {
			//driverSWLTC6804EnableBalanceResistorsArray(modPowerElectronicsPackStateHandle->cellModuleBalanceResistorEnableMask);
		}break;
		default:
			break;
	}
}

// New
void modPowerElectronicsCellMonitorsEnableBalanceResistorsArray(){
	modPowerElectronicsCellMonitorsCheckAndSolveInitState();
	
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			driverSWLTC6803EnableBalanceResistors(modPowerElectronicsPackStateHandle->cellModuleBalanceResistorEnableMask[0]);
		}break;
		case CELL_MON_LTC6804_1: {
			driverSWLTC6804EnableBalanceResistorsArray(modPowerElectronicsPackStateHandle->cellModuleBalanceResistorEnableMask);	
		}break;
		default:
			break;
	}
}

void modPowerElectronicsCellMonitorsReadVoltageFlags(uint16_t *underVoltageFlags, uint16_t *overVoltageFlags){
	modPowerElectronicsCellMonitorsCheckAndSolveInitState();
	
	switch(modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		case CELL_MON_LTC6803_2: {
			driverSWLTC6803ReadVoltageFlags(underVoltageFlags,overVoltageFlags);
		}break;
		case CELL_MON_LTC6804_1: {
			driverSWLTC6804ReadVoltageFlags(underVoltageFlags,overVoltageFlags);
		}break;
		default:
			break;
	}
}

void modPowerElectronicsCellMonitorsCheckAndSolveInitState(void){
  if(modPowerElectronicsCellMonitorsTypeActive != modPowerElectronicsGeneralConfigHandle->cellMonitorType){
		modPowerElectronicsCellMonitorsInit();
	}
}

void modPowerElectronicsTerminalCellConnectionTest(int argc, const char **argv) {
	(void)argc;
	(void)argv;
	uint32_t delayLastTick = HAL_GetTick();
	float conversionResults[5][modPowerElectronicsGeneralConfigHandle->noOfCellsSeries]; // unloaded, 100uAloaded, balance resistor load even and uneven.
	float difference;
	uint8_t cellIDPointer;
	bool passFail = true;
	bool overAllPassFail = true;
	
	float   argErrorVoltage = 1.0f;
	int32_t argBalanceDropMiliVoltage = 2;
	
	// Handle argument inputs
	modCommandsPrintf("---------     Test inputs:     ---------");
	if(argc == 3){
		// Two arguments given, taking this as balance and error threshold.
		sscanf(argv[1], "%f", &argErrorVoltage);
		sscanf(argv[2], "%d", &argBalanceDropMiliVoltage);
	}else{
	  modCommandsPrintf("No valid test arguments given. Using defaults:");
	}

	modCommandsPrintf("Error threshold:   %.1fV",argErrorVoltage);
	modCommandsPrintf("Balance threshold: %dmV",argBalanceDropMiliVoltage);
	// end of argument inputs
	
	
	// Start of general voltage test
	modCommandsPrintf("---  Starting voltage measure test  ---");	
	modCommandsPrintf("Pack voltage Direct   : %.2fV",modPowerElectronicsPackStateHandle->packVoltage);
	modCommandsPrintf("Pack voltage CVAverage: %.2fV",modPowerElectronicsPackStateHandle->cellVoltageAverage*modPowerElectronicsGeneralConfigHandle->noOfCellsSeries);
	modCommandsPrintf("Measure error         : %.2fV",fabs(modPowerElectronicsPackStateHandle->cellVoltageAverage*modPowerElectronicsGeneralConfigHandle->noOfCellsSeries-modPowerElectronicsPackStateHandle->packVoltage));
	
	if(fabs(modPowerElectronicsPackStateHandle->cellVoltageAverage*modPowerElectronicsGeneralConfigHandle->noOfCellsSeries-modPowerElectronicsPackStateHandle->packVoltage) > argErrorVoltage){
		passFail = overAllPassFail = false;
	}else{
		passFail = true;
	}
	modCommandsPrintf("Result                : %s",passFail ? "Pass" : "Fail");// Tell whether test passed / failed
	
	
	// Start of connection test
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait 
	modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	modPowerElectronicsCellMonitorsStartCellConversion();              // Start ADC conversion
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait
  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	for(cellIDPointer = 0; cellIDPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries ; cellIDPointer++){
	  conversionResults[4][cellIDPointer] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellIDPointer].cellVoltage;
	}
		
	
	modCommandsPrintf("------  Starting connectionTest  ------");
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait 
	modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	modPowerElectronicsCellMonitorsStartLoadedCellConversion(false);   // Start ADC conversion
		
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait 
	modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	modPowerElectronicsCellMonitorsStartLoadedCellConversion(false);   // Start ADC conversion
		
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait
  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	for(cellIDPointer = 0; cellIDPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries ; cellIDPointer++){
	  conversionResults[0][cellIDPointer] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellIDPointer].cellVoltage;
	}

	
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait
  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	modPowerElectronicsCellMonitorsStartLoadedCellConversion(true);    // Start ADC conversion
		
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait
  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	modPowerElectronicsCellMonitorsStartLoadedCellConversion(true);    // Start ADC conversion
		
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait
  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
	modPowerElectronicsCellMonitorsStartLoadedCellConversion(true);    // Start ADC conversion
		
	for(cellIDPointer = 0; cellIDPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries ; cellIDPointer++){
	  conversionResults[1][cellIDPointer] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellIDPointer].cellVoltage;
	}
	
	for(cellIDPointer = 0; cellIDPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries ; cellIDPointer++){
		difference = conversionResults[0][cellIDPointer] - conversionResults[1][cellIDPointer];                                          // Calculate difference
		
		if((conversionResults[0][cellIDPointer] >= modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage) || (conversionResults[0][cellIDPointer] <= modPowerElectronicsGeneralConfigHandle->cellHardUnderVoltage)) {
			overAllPassFail = passFail = false;
		}else{
			if((cellIDPointer != 0) && (cellIDPointer != 11) && (cellIDPointer != 1) && (fabs(difference) >= 0.05f))
				overAllPassFail = passFail = false;
			else
			  passFail = true;
		}
		
	  modCommandsPrintf("[%2d] %.3fV - %.3fV = % .3fV -> %s",cellIDPointer,conversionResults[0][cellIDPointer],conversionResults[1][cellIDPointer],difference,passFail ? "Pass" : "Fail");  // Print the results
	}
	modCommandsPrintf("------    End connectionTest     ------");
	modCommandsPrintf("------    Start balance test     ------");
	
	uint16_t cellBalanceMaskEnableRegister = 0;
	for(int outputPointer = 0 ; outputPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries; outputPointer++) {
		cellBalanceMaskEnableRegister |= (1 << outputPointer);
	}

	// Even cells
	modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();
	modPowerElectronicsCellMonitorsEnableBalanceResistors(cellBalanceMaskEnableRegister & 0x0AAA);
	
	for(int delay = 0; delay < 5; delay++){
		while(!modDelayTick100ms(&delayLastTick,3)){};                   // Wait
		modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	 // Read cell voltages from monitor
		modPowerElectronicsCellMonitorsStartCellConversion();            // Start ADC conversion
	}
		
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait
  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
		
	for(cellIDPointer = 0; cellIDPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries ; cellIDPointer++){
	  conversionResults[2][cellIDPointer] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellIDPointer].cellVoltage;
	}
	
	// Uneven cells
	modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();
	modPowerElectronicsCellMonitorsEnableBalanceResistors(cellBalanceMaskEnableRegister & 0x0555);
	
	
	for(int delay = 0; delay < 5; delay++){
		while(!modDelayTick100ms(&delayLastTick,3)){};                   // Wait
		modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	 // Read cell voltages from monitor
		modPowerElectronicsCellMonitorsStartCellConversion();            // Start ADC conversion
	}
		
	while(!modDelayTick100ms(&delayLastTick,3)){};                     // Wait
  modPowerElectronicsCellMonitorsCheckConfigAndReadAnalogData();	   // Read cell voltages from monitor
		
	for(cellIDPointer = 0; cellIDPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries ; cellIDPointer++){
	  conversionResults[3][cellIDPointer] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellIDPointer].cellVoltage;
	}

	modPowerElectronicsCellMonitorsEnableBalanceResistors(0);
	
	for(cellIDPointer = 0; cellIDPointer < modPowerElectronicsGeneralConfigHandle->noOfCellsSeries ; cellIDPointer++){
		float difference = fabs(conversionResults[4][cellIDPointer] - conversionResults[2][cellIDPointer]) + fabs(conversionResults[4][cellIDPointer] - conversionResults[3][cellIDPointer]);                                          // Calculate difference
		
		if(difference >= (0.001f*argBalanceDropMiliVoltage))
			passFail = true;
		else
			overAllPassFail = passFail = false;
		
	  modCommandsPrintf("[%2d] %.3fV - %.3fV = % .3fV -> %s",cellIDPointer,conversionResults[2][cellIDPointer],conversionResults[3][cellIDPointer],difference,passFail ? "Pass" : "Fail");  // Print the results
	}
	
	modCommandsPrintf("------    End balance test     ------");
	modCommandsPrintf("------     Overall: %s       ------",overAllPassFail ? "Pass" : "Fail");// Tell whether test passed / failed
}

void modPowerElectronicsSamplePackAndLCData(void) {
	float tempPackVoltage;
	
	modPowerElectrinicsSamplePackVoltage(&tempPackVoltage);
	
	if(fabs(tempPackVoltage - modPowerElectronicsGeneralConfigHandle->noOfCellsSeries*modPowerElectronicsPackStateHandle->cellVoltageAverage) < 2.0f) {    // If the error is smaller than one volt continue normal operation. 
		modPowerElectronicsPackStateHandle->packVoltage = tempPackVoltage;
		modPowerElectronicsLCSenseSample();
		modPowerElectronicsVinErrorCount = 0;																								// Reset error count.
	}else{																																								// Error in voltage measurement.
		if(modPowerElectronicsVinErrorCount++ >= VinErrorThreshold){												// Increase error count
			modPowerElectronicsVinErrorCount = VinErrorThreshold;
			// Make BMS signal error state and power down.
			modPowerElectronicsVoltageSenseError = true;
		}else{
			modPowerElectronicsLCSenseInit();																												// Reinit I2C and ISL
		}
	}
}

void modPowerElectrinicsSamplePackVoltage(float *voltagePointer) {
	switch(modPowerElectronicsGeneralConfigHandle->packVoltageDataSource) {
		case sourcePackVoltageNone:
			break;
		case sourcePackVoltageISL28022_2_0X40_LVBatteryIn:
			if(modPowerElectronicsPackStateHandle->slaveShieldPresenceMasterISL) {
				driverSWISL28022GetBusVoltage(ISL28022_MASTER_ADDRES,ISL28022_MASTER_BUS,voltagePointer,0.004f);
			}else{
				*voltagePointer = 11.22f;
			}
			break;
		case sourcePackVoltageISL28022_1_0X44_LVLoadOutput:
			if(modPowerElectronicsPackStateHandle->slaveShieldPresenceMainISL) {
				driverSWISL28022GetBusVoltage(ISL28022_SHIELD_MAIN_ADDRES,ISL28022_SHIELD_MAIN_BUS,voltagePointer,0.004f);
			}else{
				*voltagePointer = 22.11f;
			}
			break;
		case sourcePackVoltageISL28022_1_0X45_DCDC:
			if(modPowerElectronicsPackStateHandle->hiAmpShieldPresent) {
				driverSWISL28022GetBusVoltage(ISL28022_SHIELD_AUX_ADDRES,ISL28022_SHIELD_AUX_BUS,voltagePointer,0.004f);
			}else{
				*voltagePointer = 0.0f;
			}
			break;
		case sourcePackVoltageADS1015_AN01_HVBatteryIn:
			if(modPowerElectronicsPackStateHandle->hiAmpShieldPresent) {
				*voltagePointer = driverSWADS1015GetVoltage(ADS1015P0N1,0.00527083333f);
			}else{
				*voltagePointer = 0.0f;
			}
			break;
		case sourcePackVoltageADS1015_AN23_HVLoadOut:
			if(modPowerElectronicsPackStateHandle->hiAmpShieldPresent) {
				*voltagePointer = driverSWADS1015GetVoltage(ADS1015P2N3,0.00527083333f);
			}else{
				*voltagePointer = 0.0f;
			}
			break;
		case sourcePackVoltageSumOfIndividualCellVoltages:
			*voltagePointer = modPowerElectronicsGeneralConfigHandle->noOfCellsSeries*modPowerElectronicsPackStateHandle->cellVoltageAverage;
			break;
		case sourcePackVoltageCANDieBieShunt:
			*voltagePointer = 0.0f;
			break;
		case sourcePackVoltageCANIsabellenhutte:
			*voltagePointer = 0.0f;
			break;
		default:
			break;
	}
}

float modPowerElectronicsCalcPackCurrent(void){
	float returnCurrent = 0.0f;

	switch(modPowerElectronicsGeneralConfigHandle->packCurrentDataSource){
		case sourcePackCurrentLowCurrentShunt:
			returnCurrent = modPowerElectronicsPackStateHandle->loCurrentLoadCurrent;
			break;
		case sourcePackCurrentHighCurrentShunt:
			returnCurrent = modPowerElectronicsPackStateHandle->hiCurrentLoadCurrent;
			break;
		case sourcePackCurrentLowPlusHighCurrentShunt:
			returnCurrent = modPowerElectronicsPackStateHandle->loCurrentLoadCurrent + modPowerElectronicsPackStateHandle->hiCurrentLoadCurrent;
			break;
		case sourcePackCurrentNone:
		case sourcePackCurrentCANDieBieShunt:
			returnCurrent = 0.0f;
			break;
		case sourcePackCurrentCANIsaBellenHuette:
			returnCurrent = IVTCurrent;
			break;	
		default:
			break;
	}
	
	return returnCurrent;
}

void modPowerElectronicsLCSenseSample(void) {
	if(modPowerElectronicsPackStateHandle->slaveShieldPresenceMasterISL) {
		driverSWISL28022GetBusCurrent(ISL28022_MASTER_ADDRES,ISL28022_MASTER_BUS,&modPowerElectronicsPackStateHandle->loCurrentLoadCurrent,modPowerElectronicsGeneralConfigHandle->shuntLCOffset,modPowerElectronicsGeneralConfigHandle->shuntLCFactor);
		driverHWADCGetLoadVoltage(&modPowerElectronicsPackStateHandle->loCurrentLoadVoltage);
	}else{
		modPowerElectronicsPackStateHandle->loCurrentLoadVoltage = 0.0f;
		modPowerElectronicsPackStateHandle->loCurrentLoadCurrent = 0.0f;
	}
}

void modPowerElectronicsLCSenseInit(void) {
	if(modPowerElectronicsPackStateHandle->slaveShieldPresenceMasterISL) {
		modPowerElectronicsInitISL();
	}
}

uint16_t modPowerElectronicsLowestInThree(uint16_t num1,uint16_t num2,uint16_t num3) {
	if(num1 < num2 && num1 < num3) {
		return num1;
	}	else if(num2 < num3) {
		return num2;
	}	else{
		return num3;
	}
}





