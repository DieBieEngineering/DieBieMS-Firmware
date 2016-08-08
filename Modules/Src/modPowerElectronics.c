#include "modPowerElectronics.h"

// TEMP FOR EVALUATION
uint16_t cell_codes[TotalLTCICs][12]; 
/*!< 
  The cell codes will be stored in the cell_codes[][12] array in the following format:
  
  |  cell_codes[0][0]| cell_codes[0][1] |  cell_codes[0][2]|    .....     |  cell_codes[0][11]|  cell_codes[1][0] | cell_codes[1][1]|  .....   |
  |------------------|------------------|------------------|--------------|-------------------|-------------------|-----------------|----------|
  |IC1 Cell 1        |IC1 Cell 2        |IC1 Cell 3        |    .....     |  IC1 Cell 12      |IC2 Cell 1         |IC2 Cell 2       | .....    |
****/

modPowerElectricsPackStateTypedef *modPowerElectronicsPackStateHandle;
modConfigGeneralConfigStructTypedef *modPowerElectronicsGeneralConfigHandle;
uint32_t modPowerElectronicsISLIntervalLastTick;

void modPowerElectronicsInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer) {
	modPowerElectronicsGeneralConfigHandle = generalConfigPointer;
	modPowerElectronicsPackStateHandle = packState;
	
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
	modPowerElectronicsPackStateHandle->packOperationalState = PACK_STATE_NORMAL;
	
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
	
	// Init battery stack monitor
	driverLTC6803ConfigStructTypedef configStruckt;
	configStruckt.WatchDogFlag = true;																												// Don't change watchdog
	configStruckt.GPIO1 = true;
	configStruckt.GPIO2 = true;
	configStruckt.LevelPolling = true;																												// This wil make the LTC SDO high (and low when adc is busy) instead of toggling when polling for ADC ready and AD conversion finished.
	configStruckt.CDCMode = 2;																																// Comperator period = 13ms, Vres powerdown = no.
	configStruckt.DisChargeEnableMask = 0x0000;																								// Disable all discharge resistors
	configStruckt.noOfCells = modPowerElectronicsGeneralConfigHandle->noOfCells;																				// Number of cells that can cause interrupt
	configStruckt.CellVoltageConversionMode = LTC6803StartCellVoltageADCConversionAll;				// Use normal cell conversion mode, in the future -> check for lose wires on initial startup.
  configStruckt.CellUnderVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardUnderVoltage;								// Set under limit to 3V	-> This should cause error state
	configStruckt.CellOverVoltageLimit = modPowerElectronicsGeneralConfigHandle->cellHardOverVoltage;									// Set upper limit to 4.25V  -> This should cause error state
	
	driverSWLTC6803Init(configStruckt,TotalLTCICs);																						// Config the LTC6803 and start measuring
};

void modPowerElectronicsTask(void) {
	if(modDelayTick1ms(&modPowerElectronicsISLIntervalLastTick,100)) {
		driverSWISL28022GetBusCurrent(&modPowerElectronicsPackStateHandle->packCurrent);
		driverSWISL28022GetBusVoltage(&modPowerElectronicsPackStateHandle->packVoltage);
		driverHWADCGetLoadVoltage(&modPowerElectronicsPackStateHandle->loadVoltage);
		
		driverSWLTC6803ReadCellVoltages(TotalLTCICs,cell_codes); // Clean up this bit
		for(int i=0; i < modPowerElectronicsGeneralConfigHandle->noOfCells; i++)
			modPowerElectronicsPackStateHandle->cellVoltagesIndividual[0][i] = cell_codes[0][i]*0.0015; // <-- this mess
		
		driverSWLTC6803StartCellVoltageConversion();
		driverSWLTC6803ResetCellVoltageRegisters();
		
		modPowerElectronicsSubTaskBalaning();
		modPowerElectronicsSubTaskVoltageWatch();
	}
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
		
		if((modPowerElectronicsPackStateHandle->loadVoltage < PRECHARGE_PERCENTAGE*(modPowerElectronicsPackStateHandle->packVoltage)) && newState) {// Prevent turn on with to low output voltage
			return false;																																						// Load voltage to low (output not precharged enough)
		}
		
		modPowerElectronicsPackStateHandle->disChargeDesired = newState;
		modPowerElectronicsUpdateSwitches();
		dischargeLastState = newState;
		return true;
	}
	
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
		cellVoltagesSummed += modPowerElectronicsPackStateHandle->cellVoltagesIndividual[0][cellPointer];
		
		if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[0][cellPointer] > modPowerElectronicsPackStateHandle->cellVoltageHigh)
			modPowerElectronicsPackStateHandle->cellVoltageHigh = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[0][cellPointer];
		
		if(modPowerElectronicsPackStateHandle->cellVoltagesIndividual[0][cellPointer] < modPowerElectronicsPackStateHandle->cellVoltageLow)
			modPowerElectronicsPackStateHandle->cellVoltageLow = modPowerElectronicsPackStateHandle->cellVoltagesIndividual[0][cellPointer];		
	}
	
	modPowerElectronicsPackStateHandle->cellVoltageAverage = cellVoltagesSummed/modPowerElectronicsGeneralConfigHandle->noOfCells;
};

void modPowerElectronicsSubTaskBalaning(void) {
	
};

void modPowerElectronicsSubTaskVoltageWatch(void) {
	static bool lastDischargeAllowed = false;
	static bool lastChargeAllowed = false;
	uint16_t hardUnderVoltageFlags, hardOverVoltageFlags;
	
	driverSWLTC6803ReadVoltageFlags(&hardUnderVoltageFlags,&hardOverVoltageFlags);
	modPowerElectronicsCalculateCellStats();
	
	if(modPowerElectronicsPackStateHandle->packOperationalState != PACK_STATE_ERROR_HARD_CELLVOLTAGE) {
		// Handle soft cell voltage limits
		if(modPowerElectronicsPackStateHandle->cellVoltageLow <= modPowerElectronicsGeneralConfigHandle->cellSoftUnderVoltage) {
			modPowerElectronicsPackStateHandle->disChargeAllowed = false;
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageHigh >= modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage) {
			modPowerElectronicsPackStateHandle->chargeAllowed = false;
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageLow >= (modPowerElectronicsGeneralConfigHandle->cellSoftUnderVoltage + modPowerElectronicsGeneralConfigHandle->hysteresisCharge)) {
			modPowerElectronicsPackStateHandle->disChargeAllowed = true;
		}
		
		if(modPowerElectronicsPackStateHandle->cellVoltageHigh <= (modPowerElectronicsGeneralConfigHandle->cellSoftOverVoltage - modPowerElectronicsGeneralConfigHandle->hysteresisCharge)) {
			modPowerElectronicsPackStateHandle->chargeAllowed = true;
		}
		
		if(modPowerElectronicsPackStateHandle->chargeAllowed && modPowerElectronicsPackStateHandle->disChargeAllowed)
			modPowerElectronicsPackStateHandle->packOperationalState = PACK_STATE_NORMAL;
		else
			modPowerElectronicsPackStateHandle->packOperationalState = PACK_STATE_ERROR_SOFT_CELLVOLTAGE;
	}
	
	// Handle hard cell voltage limits
	if(hardUnderVoltageFlags || hardOverVoltageFlags) {
		modPowerElectronicsPackStateHandle->packOperationalState = PACK_STATE_ERROR_HARD_CELLVOLTAGE;
		modPowerElectronicsPackStateHandle->disChargeAllowed = false;
		modPowerElectronicsPackStateHandle->chargeAllowed = false;
	}
	
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

