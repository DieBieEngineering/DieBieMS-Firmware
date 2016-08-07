#include "modDelay.h"
#include "driverHWPowerState.h"

#define POWERBUTTON_THRESHOLD_TIME 1000
#define POWERBUTTON_DEBOUNCE_TIME 50

void modPowerStateInit(PowerStateStateTypedef desiredPowerState);
void modPowerStateTask(void);
bool modPowerStateButtonPressed(void);
bool modPowerStateChargerDetected(void);
bool modPowerStatePowerdownRequest(void);
void modPowerStateSetState(PowerStateStateTypedef newState);
bool modPowerStateButtonPressedOnTurnon(void);
