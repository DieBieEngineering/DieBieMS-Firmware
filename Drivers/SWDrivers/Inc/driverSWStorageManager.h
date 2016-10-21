#ifndef __DRIVERSWSTORAGEMANAGER_H
#define __DRIVERSWSTORAGEMANAGER_H

#include "stdint.h"
#include "stdbool.h"
#include "driverHWEEPROM.h"

extern bool driverSWStorageManagerConfigEmpty;
extern bool driverSWStorageManagerStateOfChargeEmpty;
extern uint16_t driverSWStorageManagerConfigStructSize;
extern uint16_t driverSWStorageManagerStateOfChargeStructSize;

void driverSWStorageManagerInit(void);
bool driverSWStorageManagerStoreConfigStruct(void *configStruct);
bool driverSWStorageManagerGetConfigStruct(void *configStruct);

#endif
