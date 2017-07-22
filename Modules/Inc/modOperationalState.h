#include "stdbool.h"
#include "modDelay.h"
#include "modPowerState.h"
#include "modEffect.h"
#include "modPowerElectronics.h"
#include "modDisplay.h"
#include "modConfig.h"
#include "modStateOfCharge.h"
#include "math.h"
#include "modMessage.h"

typedef enum {
	OP_STATE_INIT = 0,											// 0
	OP_STATE_CHARGING,											// 1
	OP_STATE_PRE_CHARGE,										// 2
	OP_STATE_LOAD_ENABLED,									// 3
	OP_STATE_BATTERY_DEAD,									// 4
	OP_STATE_POWER_DOWN,										// 5
	OP_STATE_EXTERNAL,											// 6
	OP_STATE_ERROR,													// 7
	OP_STATE_BALANCING,											// 8
	OP_STATE_CHARGED,												// 9
	OP_STATE_FORCEON,												// 10
} OperationalStateTypedef;

void modOperationalStateInit(modPowerElectricsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer,modStateOfChargeStructTypeDef *generalStateOfCharge);
void modOperationalStateTask(void);
void modOperationalStateUpdateStates(void);
void modOperationalStateSetAllStates(OperationalStateTypedef newState);
void modOperationalStateSetNewState(OperationalStateTypedef newState);
void modOperationalStateHandleChargerDisconnect(OperationalStateTypedef newState);
void modOperationalStateTerminateOperation(void);
