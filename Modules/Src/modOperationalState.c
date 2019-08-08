#include "modOperationalState.h"

OperationalStateTypedef modOperationalStateLastState;
OperationalStateTypedef modOperationalStateCurrentState;
OperationalStateTypedef modOperationalStateNewState;
modPowerElectronicsPackOperationalCellStatesTypedef packOperationalCellStateLastErrorState;
modPowerElectronicsPackStateTypedef *modOperationalStatePackStatehandle;
modConfigGeneralConfigStructTypedef *modOperationalStateGeneralConfigHandle;
modStateOfChargeStructTypeDef *modOperationalStateGeneralStateOfCharge;
modDisplayDataTypedef modOperationalStateDisplayData;
uint32_t modOperationalStateChargerTimeout;
uint32_t modOperationalStateChargedTimeout;
uint32_t modOperationalStatePreChargeTimeout;
uint32_t modOperationalStateStartupDelay;
uint32_t modOperationalStateChargerDisconnectDetectDelay;
uint32_t modOperationalStateBatteryDeadDisplayTime;
uint32_t modOperationalStateErrorDisplayTime;
uint32_t modOperationalStateNotUsedResetDelay;
uint32_t modOperationalStateNotUsedTime;
uint32_t modOperationalStatePSPDisableDelay;
uint32_t modOperationalStateWatchDogCountdownLastTick;
bool modOperationalStateForceOn;


//Private funktion predefenitions
void modOperationalStateUpdateStates(void);
void modOperationalStateSetAllStates(OperationalStateTypedef newState);
void modOperationalStateSetNewState(OperationalStateTypedef newState);

void modOperationalStateInit(modPowerElectronicsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer, modStateOfChargeStructTypeDef *generalStateOfCharge) {
	modOperationalStatePackStatehandle = packState;
	modOperationalStateGeneralConfigHandle = generalConfigPointer;
	modOperationalStateGeneralStateOfCharge = generalStateOfCharge;
	modOperationalStateSetAllStates(OP_STATE_INIT);
	modOperationalStateStartupDelay = HAL_GetTick();
	modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
	packOperationalCellStateLastErrorState = PACK_STATE_NORMAL;
	modOperationalStateForceOn = false;
	modDisplayInit();
	
	modOperationalStateNotUsedTime = HAL_GetTick();
	modOperationalStateNotUsedResetDelay = HAL_GetTick();
};

void modOperationalStateTask(void) {	
	switch(modOperationalStateCurrentState) {
		case OP_STATE_INIT:
			if(modPowerStateChargerDetected()) {																		// Check to detect charger
				switch(modOperationalStateGeneralConfigHandle->chargeEnableOperationalState){
				  case opStateChargingModeCharging:
						modOperationalStateSetNewState(OP_STATE_CHARGING);								// Go to charge state
						modEffectChangeState(STAT_LED_POWER,STAT_FLASH);									// Flash power LED when charging
						modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
						break;
					case opStateChargingModeNormal:
					default:					
						modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);							// Prepare to goto operational state
						modEffectChangeState(STAT_LED_POWER,STAT_SET);										// Turn LED on in normal operation
						break;
				}

			}else if(modOperationalStateLastState == OP_STATE_SAFESTATE){								//Just came out of safestate.
					modEffectChangeState(STAT_LED_POWER,STAT_SET);												// Turn LED on in normal operation
					//Reset variables
					modPowerElectroicsResetState();
					modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);

			}else if(modPowerStateButtonPressedOnTurnon()) {												// Check if button was pressen on turn-on
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);									// Prepare to goto operational state
				modEffectChangeState(STAT_LED_POWER,STAT_SET);												// Turn LED on in normal operation

			}else if(modOperationalStateNewState == OP_STATE_INIT){								  // USB or CAN origin of turn-on
				switch(modOperationalStateGeneralConfigHandle->externalEnableOperationalState){
					case opStateExtNormal:
						modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);							// Prepare to goto normal operational state
						break;
					case opStateExternal:
					default:
						modOperationalStateSetNewState(OP_STATE_EXTERNAL);								// Serve external control
						break;
				}
				modEffectChangeState(STAT_LED_POWER,STAT_SET);												// Turn LED on in normal operation
			}
			
			driverHWSwitchesSetSwitchState(SWITCH_DRIVER,SWITCH_SET);								// Enable FET driver.
			if(modDelayTick1ms(&modOperationalStateStartupDelay,modOperationalStateGeneralConfigHandle->displayTimeoutSplashScreen)) {// Wait for a bit than update state. Also check voltage after main fuse? followed by going to error state if blown?		
				if(!modOperationalStatePackStatehandle->disChargeLCAllowed && !modPowerStateChargerDetected()) {						// If discharge is not allowed
					modOperationalStateSetNewState(OP_STATE_BATTERY_DEAD);							// Then the battery is dead
					modOperationalStateBatteryDeadDisplayTime = HAL_GetTick();
				}
				modOperationalStateUpdateStates();																		// Sync states
			};
			
			modDisplayShowInfo(DISP_MODE_SPLASH,modOperationalStateDisplayData);
			break;
		case OP_STATE_CHARGING:
			// If chargeAllowed = false -> operational state balancing
			if(!modOperationalStatePackStatehandle->chargeAllowed)
				modOperationalStateSetNewState(OP_STATE_BALANCING);
			
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			modPowerElectronicsSetCharge(true);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_CHARGE,modOperationalStateDisplayData);
			break;
		case OP_STATE_PRE_CHARGE:
			// in case of timeout: disable pre charge & go to error state
			if(modOperationalStateLastState != modOperationalStateCurrentState) { 	  // If discharge is not allowed pre-charge will not be enabled, therefore reset timeout every task call. Also reset on first entry
				modOperationalStatePreChargeTimeout = HAL_GetTick();										// Reset timeout
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			}
		
			if(modOperationalStatePackStatehandle->disChargeLCAllowed || modOperationalStateForceOn)
				modPowerElectronicsSetPreCharge(true);
			else{
				modPowerElectronicsSetPreCharge(false);
				modOperationalStatePreChargeTimeout = HAL_GetTick();
			}
			
			if((modOperationalStatePackStatehandle->loCurrentLoadVoltage > modOperationalStatePackStatehandle->packVoltage*modOperationalStateGeneralConfigHandle->minimalPrechargePercentage) && (modOperationalStatePackStatehandle->disChargeLCAllowed || modOperationalStateForceOn)) {
				if(modOperationalStateForceOn) {
					modOperationalStateSetNewState(OP_STATE_FORCEON);								// Goto force on
				}else{
					modOperationalStateSetNewState(OP_STATE_LOAD_ENABLED);					// Goto normal load enabled operation
				}
			}else if(modDelayTick1ms(&modOperationalStatePreChargeTimeout,modOperationalStateGeneralConfigHandle->timeoutLCPreCharge)){
				if(modOperationalStateGeneralConfigHandle->LCUsePrecharge)
				  modOperationalStateSetNewState(OP_STATE_ERROR_PRECHARGE);				// An error occured during pre charge
				else
					modOperationalStateSetNewState(OP_STATE_LOAD_ENABLED);					// Goto normal load enabled operation
			}
		
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_LOAD_ENABLED:
			if(modPowerElectronicsSetDisCharge(true)) {
				modPowerElectronicsSetPreCharge(false);
			  modPowerElectronicsSetCharge(modOperationalStateGeneralConfigHandle->allowChargingDuringDischarge);
			}else{
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			}
			
			if(modPowerStateChargerDetected() && !modOperationalStateGeneralConfigHandle->allowChargingDuringDischarge) {
				modOperationalStateSetNewState(OP_STATE_INIT);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			};
			
			if(!modOperationalStatePackStatehandle->disChargeLCAllowed) {							// Battery is empty?
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			}
			
			if(fabs(modOperationalStatePackStatehandle->packCurrent) >= modOperationalStateGeneralConfigHandle->notUsedCurrentThreshold) {
				if(modDelayTick1ms(&modOperationalStateNotUsedResetDelay,1000))
					modOperationalStateNotUsedTime = HAL_GetTick();
			}else{
				modOperationalStateNotUsedResetDelay = HAL_GetTick();
			}
			
			if(modOperationalStatePowerDownDelayCheck()) {
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
			
			if(modOperationalStatePackStatehandle->chargeBalanceActive) {
				if(!modOperationalStatePackStatehandle->chargeAllowed && (modOperationalStatePackStatehandle->cellVoltageMisMatch < modOperationalStateGeneralConfigHandle->maxMismatchThreshold)){
					if(modDelayTick1ms(&modOperationalStateChargedTimeout,modOperationalStateGeneralConfigHandle->timeoutChargingCompletedMinimalMismatch)) {
						modStateOfChargeVoltageEvent(EVENT_FULL);
						modOperationalStateChargedTimeout = HAL_GetTick();
					}
				}else{
					modOperationalStateChargedTimeout = HAL_GetTick();
				};			
			}
			
			modOperationalStateUpdateStates();
			
			modOperationalStateDisplayData.StateOfCharge = modOperationalStateGeneralStateOfCharge->generalStateOfCharge;

			modDisplayShowInfo(DISP_MODE_LOAD,modOperationalStateDisplayData);
			break;
		case OP_STATE_BATTERY_DEAD:
			modDisplayShowInfo(DISP_MODE_BATTERY_DEAD,modOperationalStateDisplayData);
			if(modDelayTick1ms(&modOperationalStateBatteryDeadDisplayTime,modOperationalStateGeneralConfigHandle->displayTimeoutBatteryDead)){
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_POWER_DOWN:
			if(modOperationalStateLastState != modOperationalStateCurrentState) {
			  modOperationalStatePSPDisableDelay = HAL_GetTick();
			}
			modPowerElectronicsDisableAll();																				// Disable all power paths
			modEffectChangeState(STAT_LED_POWER,STAT_FLASH_FAST);												// Turn off power LED
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_POWEROFF,modOperationalStateDisplayData);
		  if(modDelayTick1ms(&modOperationalStatePSPDisableDelay,modOperationalStateGeneralConfigHandle->powerDownDelay))	{					// Wait for the power down delay time to pass
			  modOperationalStateTerminateOperation();															// Disable powersupply and store SoC
			}
			break;
		case OP_STATE_EXTERNAL:																										// BMS is turned on by external force IE CAN or USB
			if(modOperationalStateLastState != modOperationalStateCurrentState) {
				modPowerElectronicsSetPreCharge(false);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			}
		
			modOperationalStateTerminateOperation();																// Disable power and store SoC
			modDisplayShowInfo(DISP_MODE_EXTERNAL,modOperationalStateDisplayData);
			
			break;
		case OP_STATE_ERROR:
			// Go to save state and in the future -> try to handle error situation
			if(modOperationalStateLastState != modOperationalStateCurrentState)
				modOperationalStateErrorDisplayTime = HAL_GetTick();
			
			if(modDelayTick1ms(&modOperationalStateErrorDisplayTime,modOperationalStateGeneralConfigHandle->displayTimeoutBatteryError)) {
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
		
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			modEffectChangeState(STAT_LED_POWER,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			modPowerElectronicsDisableAll();
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_ERROR,modOperationalStateDisplayData);
			break;
		case OP_STATE_ERROR_PRECHARGE:
			// Go to save state and in the future -> try to handle error situation
			if(modOperationalStateLastState != modOperationalStateCurrentState)
				modOperationalStateErrorDisplayTime = HAL_GetTick();
			
			if(modDelayTick1ms(&modOperationalStateErrorDisplayTime,modOperationalStateGeneralConfigHandle->displayTimeoutBatteryErrorPreCharge)) {
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
		
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			modEffectChangeState(STAT_LED_POWER,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			modPowerElectronicsDisableAll();
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_ERROR_PRECHARGE,modOperationalStateDisplayData);
			break;
		case OP_STATE_BALANCING:
			// update timeout time for balancing and use charging manager for enable state charge input
			if(modOperationalStatePackStatehandle->packCurrent < modOperationalStateGeneralConfigHandle->chargerEnabledThreshold){
				if(modDelayTick1ms(&modOperationalStateChargerTimeout,modOperationalStateGeneralConfigHandle->timeoutChargeCompleted)) {
					modOperationalStateSetAllStates(OP_STATE_CHARGED);
					modStateOfChargeVoltageEvent(EVENT_FULL);
				}
			}else{
				modOperationalStateChargerTimeout = HAL_GetTick();
			};
			
			if(!modOperationalStatePackStatehandle->chargeAllowed && (modOperationalStatePackStatehandle->cellVoltageMisMatch < modOperationalStateGeneralConfigHandle->maxMismatchThreshold)){
				if(modDelayTick1ms(&modOperationalStateChargedTimeout,modOperationalStateGeneralConfigHandle->timeoutChargingCompletedMinimalMismatch)) {
					modOperationalStateSetAllStates(OP_STATE_CHARGED);
					modStateOfChargeVoltageEvent(EVENT_FULL);
				}
			}else{
				modOperationalStateChargedTimeout = HAL_GetTick();
			};
		
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			modPowerElectronicsSetCharge(true);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_BALANCING,modOperationalStateDisplayData);
			modEffectChangeState(STAT_LED_POWER,STAT_BLINKSHORTLONG_100_20);								// Indicate balancing
			break;
		case OP_STATE_CHARGED:
			// Sound the beeper indicating charging done
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			modEffectChangeState(STAT_LED_POWER,STAT_BLINKSHORTLONG_1000_4);								// Indicate Charged
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_CHARGED,modOperationalStateDisplayData);
			break;
		case OP_STATE_FORCEON:
			if(modPowerElectronicsSetDisCharge(true))
				modPowerElectronicsSetPreCharge(false);
			else {
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
				modPowerElectronicsSetDisCharge(false);
			}
						
			if(fabs(modOperationalStatePackStatehandle->packCurrent) >= modOperationalStateGeneralConfigHandle->notUsedCurrentThreshold) {
				if(modDelayTick1ms(&modOperationalStateNotUsedResetDelay,1000))
					modOperationalStateNotUsedTime = HAL_GetTick();
			}else{
				modOperationalStateNotUsedResetDelay = HAL_GetTick();
			}
			
			if(modOperationalStatePowerDownDelayCheck()) {
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
			
			modDisplayShowInfo(DISP_MODE_FORCED_ON,modOperationalStateDisplayData);
			modEffectChangeState(STAT_LED_POWER,STAT_BLINKSHORTLONG_1000_4);								// Turn flash fast on debug and power LED
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_SAFESTATE:
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH);										// Turn flash fast on debug and power LED
			modEffectChangeState(STAT_LED_POWER,STAT_FLASH);										// Turn flash fast on debug and power LED
			modPowerElectronicsDisableAll();
			modDisplayShowInfo(DISP_MODE_ERROR,modOperationalStateDisplayData);

			//Try to go out of safestate.
			if(packOperationalCellStateLastErrorState == PACK_STATE_NORMAL){
				if(modSafestateTryReset()){
					modOperationalStateSetNewState(OP_STATE_INIT);
					modOperationalStateUpdateStates();
					modEffectChangeState(STAT_LED_POWER,STAT_SET);
					modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH);
				}
			}
			break;

		default:
			modOperationalStateSetAllStates(OP_STATE_ERROR);
			break;
	};
	
	if(modPowerStateForceOnRequest()){
		modOperationalStateForceOn = true;
		modPowerElectronicsAllowForcedOn(true);
		modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
		driverSWStorageManagerEraseData();
	};
	
	// Check for power button longpress -> if so power down BMS
	if(modPowerStatePowerdownRequest()) {
		modOperationalStatePackStatehandle->powerDownDesired = true;
		
		if(modOperationalStateDelayedDisable(modOperationalStateGeneralConfigHandle->useCANDelayedPowerDown)) {
			modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
			modDisplayShowInfo(DISP_MODE_POWEROFF,modOperationalStateDisplayData);
			modOperationalStateUpdateStates();
		}
	};
	
	// In case of extreme cellvoltages goto error state
	// TODO also check on cell temperature and current.
	if((modOperationalStatePackStatehandle->packOperationalCellState == PACK_STATE_ERROR_HARD_CELLVOLTAGE) && (modOperationalStatePackStatehandle->packOperationalCellState != packOperationalCellStateLastErrorState) && !modOperationalStateForceOn){
		packOperationalCellStateLastErrorState = modOperationalStatePackStatehandle->packOperationalCellState; // Meganism to make error situation only trigger once
		modOperationalStateSetNewState(OP_STATE_ERROR);														// TODO: show error message then power down
		modOperationalStateUpdateStates();
	}
	
	//In case of safestate flipflop active go to software safestate.
	if(modSafeStateCheckSafestate()){
		if((modOperationalStateCurrentState != OP_STATE_SAFESTATE) && (modOperationalStateCurrentState != OP_STATE_POWER_DOWN)){
			modOperationalStateSetNewState(OP_STATE_SAFESTATE);
			modOperationalStateUpdateStates();
		}
	}

	// Move the button pressed state to the status struct
	modOperationalStatePackStatehandle->powerOnLongButtonPress = modPowerStateGetLongButtonPressState(); 
	
	// Handle subtask-display to update display content
	modDisplayTask();
};

void modOperationalStateUpdateStates(void) {
	modOperationalStateLastState = modOperationalStateCurrentState;
	modOperationalStatePackStatehandle->operationalState = modOperationalStateCurrentState = modOperationalStateNewState;
};

void modOperationalStateSetAllStates(OperationalStateTypedef newState) {
	modOperationalStatePackStatehandle->operationalState = modOperationalStateLastState = modOperationalStateCurrentState = modOperationalStateNewState = newState;
};

void modOperationalStateSetNewState(OperationalStateTypedef newState) {
	modOperationalStateNewState = newState;
};

void modOperationalStateHandleChargerDisconnect(OperationalStateTypedef newState) {
	if((modPowerStateChargerDetected() && !(modOperationalStatePackStatehandle->chargeDesired && modOperationalStatePackStatehandle->chargeAllowed)) || ((modOperationalStatePackStatehandle->packCurrent > modOperationalStateGeneralConfigHandle->chargerEnabledThreshold ) && modOperationalStatePackStatehandle->chargeDesired && modOperationalStatePackStatehandle->chargeAllowed)) {
		modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
	}else{
		if(modDelayTick1ms(&modOperationalStateChargerDisconnectDetectDelay,modOperationalStateGeneralConfigHandle->timeoutChargerDisconnected)){
			modOperationalStateSetAllStates(newState);
			modOperationalStatePackStatehandle->powerDownDesired = true;
		}
	}
};

void modOperationalStateTerminateOperation(void) {	
	// Store the state of charge data
	modStateOfChargePowerDownSave();																						// Store the SoC data
	
	// Disable the power supply
	modPowerStateSetState(P_STAT_RESET);																				// Turn off the power
}

bool modOperationalStateDelayedDisable(bool delayedPowerDownDesired) {
	if(delayedPowerDownDesired){
		if(modOperationalStatePackStatehandle->watchDogTime){
			if(modDelayTick1ms(&modOperationalStateWatchDogCountdownLastTick,1000))
				modOperationalStatePackStatehandle->watchDogTime--;
			
			return false;
		}else{
			return true;
		}
	}else{
	  return true;
	}
}

bool modOperationalStatePowerDownDelayCheck(void){
	return modDelayTick1ms(&modOperationalStateNotUsedTime,modOperationalStateGeneralConfigHandle->notUsedTimeout) && modOperationalStateGeneralConfigHandle->notUsedTimeout;
}
