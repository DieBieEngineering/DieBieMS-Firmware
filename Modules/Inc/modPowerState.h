#include "modDelay.h"
#include "driverHWPowerState.h"

#define POWERBUTTON_POWERDOWN_THRESHOLD_TIME 1000
#define POWERBUTTON_FORCEON_THRESHOLD_TIME 5000
#define POWERBUTTON_DEBOUNCE_TIME 50

void modPowerStateInit(PowerStateStateTypedef desiredPowerState);
void modPowerStateTask(void);
bool modPowerStateButtonPressed(void);
bool modPowerStateChargerDetected(void);
bool modPowerStatePowerdownRequest(void);
bool modPowerStateForceOnRequest(void);
void modPowerStateSetState(PowerStateStateTypedef newState);
bool modPowerStateButtonPressedOnTurnon(void);
