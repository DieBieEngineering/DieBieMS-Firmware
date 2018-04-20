#include "modPowerElectronics.h"

modPowerElectricsPackStateTypedef *modPowerElectronicsPackStateHandle;
modConfigGeneralConfigStructTypedef *modPowerElectronicsGeneralConfigHandle;
uint32_t modPowerElectronicsISLIntervalLastTick;

uint32_t modPowerElectronicsChargeRetryLastTick;
uint32_t modPowerElectronicsDisChargeRetryLastTick;
uint32_t modPowerElectronicsCellBalanceUpdateLastTick;
uint32_t modPowerElectronicsTempMeasureDelayLastTick;
uint8_t  modPowerElectronicsUnderAndOverVoltageErrorCount;
driverLTC6803ConfigStructTypedef modPowerElectronicsLTCconfigStruct;
bool     modPowerElectronicsAllowForcedOnState;
float    modPowerElectronicsCellVoltagesTemp[12];
uint16_t modPowerElectronicsTemperatureArray[3];

uint16_t tempTemperature;

void modPowerElectronicsInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer) {
	modPowerElectronicsGeneralConfigHandle = generalConfigPointer;
	modPowerElectronicsPackStateHandle = packState;
	modPowerElectronicsUnderAndOverVoltageErrorCount = 0;
	modPowerElectronicsAllowForcedOnState = false;
	
	// Init pack status
	modPowerElectronicsPackStateHandle->throttleDutyCharge       = 100;
	modPowerElectronicsPackStateHandle->throttleDutyDischarge    = 100;
	modPowerElectronicsPackStateHandle->packVoltage              = 0.0f;
	modPowerElectronicsPackStateHandle->packCurrent              = 0.0f;
	modPowerElectronicsPackStateHandle->loadVoltage              = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageHigh          = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageLow           = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageAverage       = 0.0;
	modPowerElectronicsPackStateHandle->disChargeDesired         = false;
	modPowerElectronicsPackStateHandle->disChargeAllowed         = true;
	modPowerElectronicsPackStateHandle->preChargeDesired         = false;
	modPowerElectronicsPackStateHandle->chargeDesired            = false;
	modPowerElectronicsPackStateHandle->chargeAllowed 					 = true;
	modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_NORMAL;
	modPowerElectronicsPackStateHandle->temperatures[0]          = 0.0f;
	modPowerElectronicsPackStateHandle->temperatures[1]          = 0.0f;
	modPowerElectronicsPackStateHandle->temperatures[2]          = 0.0f;
	modPowerElectronicsPackStateHandle->temperatures[3]          = 0.0f;
	modPowerElectronicsPackStateHandle->tempBatteryHigh          = 0.0f;
	modPowerElectronicsPackStateHandle->tempBatteryLow           = 0.0f;
	modPowerElectronicsPackStateHandle->tempBatteryAverage       = 0.0f;
	modPowerElectronicsPackStateHandle->tempBMSHigh              = 0.0f;
	modPowerElectronicsPackStateHandle->tempBMSLow               = 0.0f;
	modPowerElectronicsPackStateHandle->tempBMSAverage           = 0.0f;
	
	// Init BUS monitor
	driverSWISL28022InitStruct ISLInitStruct;
	ISLInitStruct.ADCSetting = ADC_128_64010US;
	ISLInitStruct.busVoltageRange = BRNG_60V_1;
	ISLInitStruct.currentShuntGain = PGA_4_160MV;
	ISLInitStruct.Mode = MODE_SHUNTANDBUS_CONTINIOUS;
	driverSWISL28022Init(ISL28022_MASTER_ADDRES,ISL28022_MASTER_BUS,ISLInitStruct);
	
	// Init internal ADC
	driverHWADCInit();
	driverHWSwitchesInit();
	driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);																// Enable FET Driver
	
	// Init battery stack monitor
	driverLTC6803ConfigStructTypedef configStruct;
	configStruct.WatchDogFlag = false;																											// Don't change watchdog
	configStruct.GPIO1 = false;
	configStruct.GPIO2 = true;
	configStruct.LevelPolling = true;																												// This wil make the LTC SDO high (and low when adc is busy) instead of toggling when polling for ADC ready and AD conversion finished.
	configStruct.CDCMode = 2;																																// Comperator period = 13ms, Vres powerdown = no.
	configStruct.DisChargeEnableMask = 0x0000;																							// Disable all discharge resistors
	configStruct.noOfCells = modPowerElectronicsGeneralConfigHandle->noOfCells;							// Number of cells that can cause interrupt
	configStruct.CellVoltageConversionMode = LTC6803StartCellVoltageADCConversionAll;				// Use normal cell conversion mode, in the future -> check for lose wires on initial startup.
  configStruct.CellUnderVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardUnderVoltage;	// Set under limit to XV	-> This should cause error state
	configStruct.CellOverVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage;		// Set upper limit to X.XXV  -> This should cause error state
	
	driverSWLTC6803Init(configStruct,TotalLTCICs);																					// Config the LTC6803 and start measuring
};

bool modPowerElectronicsTask(void) {
	bool returnValue = false;
	
	if(modDelayTick1ms(&modPowerElectronicsISLIntervalLastTick,100)) {
		// reset tick for LTC Temp start conversion delay
		modPowerElectronicsTempMeasureDelayLastTick = HAL_GetTick();
		
		// Collect main current path data
		driverSWISL28022GetBusCurrent(ISL28022_MASTER_ADDRES,ISL28022_MASTER_BUS,&modPowerElectronicsPackStateHandle->packCurrent,-0.004494f);
		driverSWISL28022GetBusVoltage(ISL28022_MASTER_ADDRES,ISL28022_MASTER_BUS,&modPowerElectronicsPackStateHandle->packVoltage,0.004f);
		driverHWADCGetLoadVoltage(&modPowerElectronicsPackStateHandle->loadVoltage);
		
		// Check if LTC is still running
		driverSWLTC6803ReadConfig(&modPowerElectronicsLTCconfigStruct);
		if(!modPowerElectronicsLTCconfigStruct.CDCMode)
			driverSWLTC6803ReInit();																														// Something went wrong, reinit the battery stack monitor.
		else
			driverSWLTC6803ReadCellVoltages(modPowerElectronicsPackStateHandle->cellVoltagesIndividual);
		
		// Check if LTC has discharge resistor enabled while not charging
		if(!modPowerElectronicsPackStateHandle->chargeDesired && modPowerElectronicsLTCconfigStruct.DisChargeEnableMask)
			driverSWLTC6803ReInit();																														// Something went wrong, reinit the battery stack monitor.
		
		// Collect LTC temperature data
		driverSWLTC6803ReadTempVoltages(modPowerElectronicsTemperatureArray);
		modPowerElectronicsPackStateHandle->temperatures[0] = driverSWLTC6803ConvertTemperatureExt(modPowerElectronicsTemperatureArray[0],modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupLTCExt],25.0f);
		modPowerElectronicsPackStateHandle->temperatures[1] = driverSWLTC6803ConvertTemperatureExt(modPowerElectronicsTemperatureArray[1],modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupLTCExt],modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupLTCExt],25.0f);
		modPowerElectronicsPackStateHandle->temperatures[2] = driverSWLTC6803ConvertTemperatureInt(modPowerElectronicsTemperatureArray[2]);
		// get STM32 ADC NTC temp
		driverHWADCGetNTCValue(&modPowerElectronicsPackStateHandle->temperatures[3],modPowerElectronicsGeneralConfigHandle->NTC25DegResistance[modConfigNTCGroupMasterPCB],modPowerElectronicsGeneralConfigHandle->NTCTopResistor[modConfigNTCGroupMasterPCB],modPowerElectronicsGeneralConfigHandle->NTCBetaFactor[modConfigNTCGroupMasterPCB],25.0f);
		
		// Calculate temperature statisticks
		modPowerElectronicsCalcTempStats();
		
		// Do the balancing task
		modPowerElectronicsSubTaskBalaning();
		
		// Measure cell voltages
		driverSWLTC6803StartCellVoltageConversion();
		driverSWLTC6803ResetCellVoltageRegisters();
		
		// Check and respond to the measured voltage values
		modPowerElectronicsSubTaskVoltageWatch();
		
		// Check and respond to the measured temperature values
		// modPowerElectronicsSubTaskTemperatureWatch();
		
		returnValue = true;
	}else
		returnValue = false;
	
	if(modDelayTick1msNoRST(&modPowerElectronicsTempMeasureDelayLastTick,50))
		driverSWLTC6803StartTemperatureVoltageConversion();
	
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
	
	if(modPowerElectronicsPackStateHandle->loadVoltage < PRECHARGE_PERCENTAGE*(modPowerElectronicsPackStateHandle->packVoltage)) // Prevent turn on with to low output voltage
		return false;																																						                                   // Load voltage to low (output not precharged enough)
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
	
	for(uint8_t cellPointer = 0; cellPointer < modPowerElectronicsGeneralConfigHandle->noOfCells; cellPointer++) {
		cellVoltagesSummed += modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;
		
		if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage > modPowerElectronicsPackStateHandle->cellVoltageHigh)
			modPowerElectronicsPackStateHandle->cellVoltageHigh = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;
		
		if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage < modPowerElectronicsPackStateHandle->cellVoltageLow)
			modPowerElectronicsPackStateHandle->cellVoltageLow = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[cellPointer].cellVoltage;		
	}
	
	modPowerElectronicsPackStateHandle->cellVoltageAverage = cellVoltagesSummed/modPowerElectronicsGeneralConfigHandle->noOfCells;
	modPowerElectronicsPackStateHandle->cellVoltageMisMatch = modPowerElectronicsPackStateHandle->cellVoltageHigh - modPowerElectronicsPackStateHandle->cellVoltageLow;
};

void modPowerElectronicsSubTaskBalaning(void) {
	static uint32_t delayTimeHolder = 100;
	static uint16_t lastCellBalanceRegister = 0;
	static bool delaytoggle = false;
	uint16_t cellBalanceMaskEnableRegister = 0;
	
	if(modDelayTick1ms(&modPowerElectronicsCellBalanceUpdateLastTick,delayTimeHolder)) {
		delaytoggle ^= true;
		delayTimeHolder = delaytoggle ? modPowerElectronicsGeneralConfigHandle->cellBalanceUpdateInterval : 200;
		
		if(delaytoggle) {
			for(int k=0; k<12; k++)
				modPowerElectronicsCellVoltagesTemp[k] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[k].cellVoltage;	// This will contain the voltages that are unloaded by balance resistors
			
			modPowerElectronicsSortCells(modPowerElectronicsPackStateHandle->cellVoltagesIndividual);
			
			if(modPowerElectronicsPackStateHandle->chargeDesired) {																							// Check if charging is desired
				for(uint8_t i = 0; i < modPowerElectronicsGeneralConfigHandle->maxSimultaneousDischargingCells; i++) {
					if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[i].cellVoltage >= (modPowerElectronicsPackStateHandle->cellVoltageLow + modPowerElectronicsGeneralConfigHandle->cellBalanceDifferenceThreshold)) {
						// This cell voltage should be lowered if the voltage is above threshold:
						if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[i].cellVoltage >= modPowerElectronicsGeneralConfigHandle->cellBalanceStart)
							cellBalanceMaskEnableRegister |= (1 << modPowerElectronicsPackStateHandle->cellVoltagesIndividual[i].cellNumber);
					};
				}
			}
		}
		
		modPowerElectronicsPackStateHandle->cellBalanceResistorEnableMask = cellBalanceMaskEnableRegister;
		
		if(lastCellBalanceRegister != cellBalanceMaskEnableRegister)
			driverSWLTC6803EnableBalanceResistors(cellBalanceMaskEnableRegister);
		lastCellBalanceRegister = cellBalanceMaskEnableRegister;
	}
};

void modPowerElectronicsSubTaskVoltageWatch(void) {
	static bool lastDischargeAllowed = false;
	static bool lastChargeAllowed = false;
	uint16_t hardUnderVoltageFlags, hardOverVoltageFlags;
	
	driverSWLTC6803ReadVoltageFlags(&hardUnderVoltageFlags,&hardOverVoltageFlags);
	modPowerElectronicsCalculateCellStats();
	
	if(modPowerElectronicsPackStateHandle->packOperationalCellState != PACK_STATE_ERROR_HARD_CELLVOLTAGE) {
		// Handle soft cell voltage limits
		if(modPowerElectronicsPackStateHandle->cellVoltageLow <= modPowerElectronicsGeneralConfigHandle->cellSoftUnderVoltage) {
			modPowerElectronicsPackStateHandle->disChargeAllowed = false;
			modPowerElectronicsDisChargeRetryLastTick = HAL_GetTick();
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageHigh >= modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage) {
			modPowerElectronicsPackStateHandle->chargeAllowed = false;
			modPowerElectronicsChargeRetryLastTick = HAL_GetTick();
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageLow >= (modPowerElectronicsGeneralConfigHandle->cellSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->hysteresisDischarge)) {
			if(modDelayTick1ms(&modPowerElectronicsDisChargeRetryLastTick,modPowerElectronicsGeneralConfigHandle->timeoutDischargeRetry))
				modPowerElectronicsPackStateHandle->disChargeAllowed = true;
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageHigh <= (modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->hysteresisCharge)) {
			if(modDelayTick1ms(&modPowerElectronicsChargeRetryLastTick,modPowerElectronicsGeneralConfigHandle->timeoutChargeRetry))
				modPowerElectronicsPackStateHandle->chargeAllowed = true;
		}
		
		if(modPowerElectronicsPackStateHandle->chargeAllowed && modPowerElectronicsPackStateHandle->disChargeAllowed)
			modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_NORMAL;
		else
			modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_ERROR_SOFT_CELLVOLTAGE;
	}
	
	// Handle hard cell voltage limits
	if(hardUnderVoltageFlags || hardOverVoltageFlags || (modPowerElectronicsPackStateHandle->packVoltage > modPowerElectronicsGeneralConfigHandle->noOfCells*modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage)) {
		if(modPowerElectronicsUnderAndOverVoltageErrorCount++ > modPowerElectronicsGeneralConfigHandle->maxUnderAndOverVoltageErrorCount)
			modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_ERROR_HARD_CELLVOLTAGE;
		modPowerElectronicsPackStateHandle->disChargeAllowed = false;
		modPowerElectronicsPackStateHandle->chargeAllowed = false;
	}else
		modPowerElectronicsUnderAndOverVoltageErrorCount = 0;
	
	
	// update outputs directly if needed
	if((lastChargeAllowed != modPowerElectronicsPackStateHandle->chargeAllowed) || (lastDischargeAllowed != modPowerElectronicsPackStateHandle->disChargeAllowed)) {
		lastChargeAllowed = modPowerElectronicsPackStateHandle->chargeAllowed;
		lastDischargeAllowed = modPowerElectronicsPackStateHandle->disChargeAllowed;
		modPowerElectronicsUpdateSwitches();
	}
};

// Update switch states, should be called after every desired/allowed switch state change
void modPowerElectronicsUpdateSwitches(void) {
	// Do the actual power switching in here
	
	//Handle pre charge output
	if(modPowerElectronicsPackStateHandle->preChargeDesired && (modPowerElectronicsPackStateHandle->disChargeAllowed || modPowerElectronicsAllowForcedOnState))
		driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE,(driverHWSwitchesStateTypedef)SWITCH_SET);
	else
		driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE,(driverHWSwitchesStateTypedef)SWITCH_RESET);
	
	//Handle discharge output
	if(modPowerElectronicsPackStateHandle->disChargeDesired && (modPowerElectronicsPackStateHandle->disChargeAllowed || modPowerElectronicsAllowForcedOnState))
		driverHWSwitchesSetSwitchState(SWITCH_DISCHARGE,(driverHWSwitchesStateTypedef)SWITCH_SET);
	else
		driverHWSwitchesSetSwitchState(SWITCH_DISCHARGE,(driverHWSwitchesStateTypedef)SWITCH_RESET);
	
	//Handle charge input
	if(modPowerElectronicsPackStateHandle->chargeDesired && modPowerElectronicsPackStateHandle->chargeAllowed)
		driverHWSwitchesSetSwitchState(SWITCH_CHARGE,(driverHWSwitchesStateTypedef)SWITCH_SET);
	else
		driverHWSwitchesSetSwitchState(SWITCH_CHARGE,(driverHWSwitchesStateTypedef)SWITCH_RESET);
};

void modPowerElectronicsSortCells(driverLTC6803CellsTypedef cells[12]) {
	int i,j;
	driverLTC6803CellsTypedef value;

	for(i=0;i<12-1;i++) {
		for(j=0;j<12-i-1;j++) {
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

int32_t modPowerElectronicsMapVariable(int32_t inputVariable, int32_t inputLowerLimit, int32_t inputUpperLimit, int32_t outputLowerLimit, int32_t outputUpperLimit) {
	inputVariable = inputVariable < inputLowerLimit ? inputLowerLimit : inputVariable;
	inputVariable = inputVariable > inputUpperLimit ? inputUpperLimit : inputVariable;
	
	return (inputVariable - inputLowerLimit) * (outputUpperLimit - outputLowerLimit) / (inputUpperLimit - inputLowerLimit) + outputLowerLimit;
}
