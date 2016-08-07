#include "modPowerElectronics.h"

// TEMP FOR EVALUATION
uint16_t cell_codes[TotalLTCICs][12]; 
/*!< 
  The cell codes will be stored in the cell_codes[][12] array in the following format:
  
  |  cell_codes[0][0]| cell_codes[0][1] |  cell_codes[0][2]|    .....     |  cell_codes[0][11]|  cell_codes[1][0] | cell_codes[1][1]|  .....   |
  |------------------|------------------|------------------|--------------|-------------------|-------------------|-----------------|----------|
  |IC1 Cell 1        |IC1 Cell 2        |IC1 Cell 3        |    .....     |  IC1 Cell 12      |IC2 Cell 1         |IC2 Cell 2       | .....    |
****/

modPowerElectricsPackStateTypedef *packStatePowerElectronicshandle;
uint32_t modPowerElectronicsISLIntervalLastTick;

void modPowerElectronicsInit(modPowerElectricsPackStateTypedef *packState) {
	packStatePowerElectronicshandle = packState;
	
	// Init pack status
	packStatePowerElectronicshandle->packVoltage = 0.0f;
	packStatePowerElectronicshandle->packCurrent = 0.0f;
	packStatePowerElectronicshandle->loadVoltage = 0.0f;
	packStatePowerElectronicshandle->cellVoltageHigh = 0.0f;
	packStatePowerElectronicshandle->cellVoltageLow = 0.0f;
	packStatePowerElectronicshandle->cellVoltageAverage = 0.0;
	packStatePowerElectronicshandle->disChargeAllowed = false;
	packStatePowerElectronicshandle->preChargeAllowed = false;
	packStatePowerElectronicshandle->chargeAllowed = false;
	packStatePowerElectronicshandle->packOperationalState = PACK_STATE_NORMAL;
	
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
	configStruckt.noOfCells = NoOfCells;																											// Number of cells that can cause interrupt
	configStruckt.CellVoltageConversionMode = LTC6803StartCellVoltageADCConversionAll;				// Use normal cell conversion mode, in the future -> check for lose wires on initial startup.
  configStruckt.CellUnderVoltageLimit = 3.0;																								// Set under limit to 3V	-> This should cause error state
	configStruckt.CellOverVoltageLimit = 4.25;																								// Set upper limit to 4.25V  -> This should cause error state
	
	driverSWLTC6803Init(configStruckt,TotalLTCICs);																						// Config the LTC6803 and start measuring
};

void modPowerElectronicsTask(void) {
	if(modDelayTick1ms(&modPowerElectronicsISLIntervalLastTick,100)) {
		driverSWISL28022GetBusCurrent(&packStatePowerElectronicshandle->packCurrent);
		driverSWISL28022GetBusVoltage(&packStatePowerElectronicshandle->packVoltage);
		driverHWADCGetLoadVoltage(&packStatePowerElectronicshandle->loadVoltage);
		
		driverSWLTC6803ReadCellVoltages(TotalLTCICs,cell_codes); // Clean up this bit
		for(int i=0; i<NoOfCells; i++)
			packStatePowerElectronicshandle->cellVoltagesIndividual[0][i] = cell_codes[0][i]*0.0015; // <-- this mess
		
		driverSWLTC6803StartCellVoltageConversion();
		driverSWLTC6803ResetCellVoltageRegisters();
		
		modPowerElectronicsSubTaskBalaning();
		modPowerElectronicsSubTaskVoltageWatch();
	}
};

void modPowerElectronicsSetPreCharge(bool newState) {
	if(newState)
		driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);
	driverHWSwitchesSetSwitchState(SWITCH_PRECHARGE,(driverHWSwitchesStateTypedef)newState);
};

bool modPowerElectronicsSetDisCharge(bool newState) {
	if(newState)
		driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET); 
	
	if((packStatePowerElectronicshandle->loadVoltage < PRECHARGE_PERCENTAGE*(packStatePowerElectronicshandle->packVoltage)) && newState) {// Prevent turn on with to low output voltage
		return false;																																						// Load voltage to low (output not precharged enough)
	}
	
	driverHWSwitchesSetSwitchState(SWITCH_DISCHARGE,(driverHWSwitchesStateTypedef)newState);	// Allow turnon
	return true;
};

void modPowerElectronicsSetCharge(bool newState) {
	if(newState)
		driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);
	driverHWSwitchesSetSwitchState(SWITCH_CHARGE,(driverHWSwitchesStateTypedef)newState);
};

void modPowerElectronicsDisableAll(void) {
	driverHWSwitchesDisableAll();
};

void modPowerElectronicsCalculateCellStats(void) {
	float cellVoltagesSummed = 0.0f;
	packStatePowerElectronicshandle->cellVoltageHigh = 0.0f;
	packStatePowerElectronicshandle->cellVoltageLow = 10.0f;
	
	for(uint8_t cellPointer = 0; cellPointer < NoOfCells; cellPointer++) {
		cellVoltagesSummed += packStatePowerElectronicshandle->cellVoltagesIndividual[0][cellPointer];
		
		if(packStatePowerElectronicshandle->cellVoltagesIndividual[0][cellPointer] > packStatePowerElectronicshandle->cellVoltageHigh)
			packStatePowerElectronicshandle->cellVoltageHigh = packStatePowerElectronicshandle->cellVoltagesIndividual[0][cellPointer];
		
		if(packStatePowerElectronicshandle->cellVoltagesIndividual[0][cellPointer] < packStatePowerElectronicshandle->cellVoltageLow)
			packStatePowerElectronicshandle->cellVoltageLow = packStatePowerElectronicshandle->cellVoltagesIndividual[0][cellPointer];		
	}
	
	packStatePowerElectronicshandle->cellVoltageAverage = cellVoltagesSummed/NoOfCells;
};

void modPowerElectronicsSubTaskBalaning(void) {
	
};

void modPowerElectronicsSubTaskVoltageWatch(void) {
	modPowerElectronicsCalculateCellStats();
	// Handle soft cell voltage limits
	// Handle hard cell voltage limits
	
	// update outputs directly if needed
	// modPowerElectronicsUpdateSwitches();
};

void modPowerElectronicsUpdateSwitches(void) {
	// Do the actual power switching in here
};

