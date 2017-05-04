#include "modPowerElectronics.h"

modPowerElectricsPackStateTypedef *modPowerElectronicsPackStateHandle;
modConfigGeneralConfigStructTypedef *modPowerElectronicsGeneralConfigHandle;
uint32_t modPowerElectronicsISLIntervalLastTick;

uint32_t modPowerElectronicsChargeRetryLastTick;
uint32_t modPowerElectronicsDisChargeRetryLastTick;
uint32_t modPowerElectronicsCellBalanceUpdateLastTick;
uint8_t modPowerElectronicsUnderAndOverVoltageErrorCount;
uint16_t balanceResistorMask;

driverLTC6803ConfigStructTypedef configStruct; // Temp -> delete this

float cellVoltagesTemp[12];

void modPowerElectronicsInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer) {
	modPowerElectronicsGeneralConfigHandle = generalConfigPointer;
	modPowerElectronicsPackStateHandle = packState;
	modPowerElectronicsUnderAndOverVoltageErrorCount = 0;
	
	// Init pack status
	modPowerElectronicsPackStateHandle->packVoltage = 0.0f;
	modPowerElectronicsPackStateHandle->packCurrent = 0.0f;
	modPowerElectronicsPackStateHandle->loadVoltage = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageHigh = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageLow = 0.0f;
	modPowerElectronicsPackStateHandle->cellVoltageAverage = 0.0;
	modPowerElectronicsPackStateHandle->disChargeDesired = false;
	modPowerElectronicsPackStateHandle->disChargeAllowed = true;
	modPowerElectronicsPackStateHandle->preChargeDesired = false;
	modPowerElectronicsPackStateHandle->chargeDesired = false;
	modPowerElectronicsPackStateHandle->chargeAllowed = true;
	modPowerElectronicsPackStateHandle->packOperationalCellState = PACK_STATE_NORMAL;
	
	// Init BUS monitor
	driverSWISL28022InitStruct ISLInitStruct;
	ISLInitStruct.ADCSetting = ADC_128_64010US;
	ISLInitStruct.busVoltageRange = BRNG_60V_1;
	ISLInitStruct.currentShuntGain = PGA_4_160MV;
	ISLInitStruct.Mode = MODE_SHUNTANDBUS_CONTINIOUS;
	driverSWISL28022Init(ISLInitStruct);
	
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
  configStruct.CellUnderVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardUnderVoltage;	// Set under limit to 3V	-> This should cause error state
	configStruct.CellOverVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage;		// Set upper limit to 4.25V  -> This should cause error state
	
	driverSWLTC6803Init(configStruct,TotalLTCICs);																					// Config the LTC6803 and start measuring
};

bool modPowerElectronicsTask(void) {
	if(modDelayTick1ms(&modPowerElectronicsISLIntervalLastTick,100)) {
		driverSWISL28022GetBusCurrent(&modPowerElectronicsPackStateHandle->packCurrent);
		driverSWISL28022GetBusVoltage(&modPowerElectronicsPackStateHandle->packVoltage);
		driverHWADCGetLoadVoltage(&modPowerElectronicsPackStateHandle->loadVoltage);
		
		// Check if LTC is still running
		driverSWLTC6803ReadConfig(&configStruct);
		if(!configStruct.CDCMode)
			driverSWLTC6803ReInit();																														// Something went wrong, reinit the battery stack monitor.
		else
			driverSWLTC6803ReadCellVoltages(modPowerElectronicsPackStateHandle->cellVoltagesIndividual);
		
		// Check if LTC has discharge resistor enabled while not charging
		if(!modPowerElectronicsPackStateHandle->chargeDesired && configStruct.DisChargeEnableMask)
			driverSWLTC6803ReInit();																														// Something went wrong, reinit the battery stack monitor.
		
		modPowerElectronicsSubTaskBalaning();
		
		driverSWLTC6803StartCellVoltageConversion();
		driverSWLTC6803ResetCellVoltageRegisters();
		
		modPowerElectronicsSubTaskVoltageWatch();
		
		return true;
	}else
		return false;
};

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
		return false;																																						// Load voltage to low (output not precharged enough)
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
	
	//if(modPowerElectronicsPackStateHandle->cellVoltageHigh > 5.0f)
	//	modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);
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
				cellVoltagesTemp[k] = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[k].cellVoltage;	// This will contain the voltages that are unloaded by balance resistors
			
			modPowerElectronicsSortCells(modPowerElectronicsPackStateHandle->cellVoltagesIndividual);
			
			if(modPowerElectronicsPackStateHandle->chargeDesired) {																// Check if charging is desired
				for(uint8_t i = 0; i < modPowerElectronicsGeneralConfigHandle->maxSimultaneousDischargingCells; i++) {
					if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[i].cellVoltage >= (modPowerElectronicsPackStateHandle->cellVoltageLow + modPowerElectronicsGeneralConfigHandle->cellBalanceDifferenceThreshold)) {
						// This cell voltage should be lowered if the voltage is above threshold:
						if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[i].cellVoltage >= modPowerElectronicsGeneralConfigHandle->cellBalanceStart)
							cellBalanceMaskEnableRegister |= (1 << modPowerElectronicsPackStateHandle->cellVoltagesIndividual[i].cellNumber);
					};
				}
			}
		}
		
		balanceResistorMask = cellBalanceMaskEnableRegister;
		
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
			if(modDelayTick1ms(&modPowerElectronicsDisChargeRetryLastTick,modPowerElectronicsGeneralConfigHandle->timoutDischargeRetry))
				modPowerElectronicsPackStateHandle->disChargeAllowed = true;
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageHigh <= (modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->hysteresisCharge)) {
			if(modDelayTick1ms(&modPowerElectronicsChargeRetryLastTick,modPowerElectronicsGeneralConfigHandle->timoutChargeRetry))
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
	if(modPowerElectronicsPackStateHandle->preChargeDesired && modPowerElectronicsPackStateHandle->disChargeAllowed)
		driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE,(driverHWSwitchesStateTypedef)SWITCH_SET);
	else
		driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE,(driverHWSwitchesStateTypedef)SWITCH_RESET);
	
	//Handle discharge output
	if(modPowerElectronicsPackStateHandle->disChargeDesired && modPowerElectronicsPackStateHandle->disChargeAllowed)
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
