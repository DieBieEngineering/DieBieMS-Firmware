#ifndef __DRIVERSWSTORAGEMANAGER_H
#define __DRIVERSWSTORAGEMANAGER_H

#include "stdint.h"
#include "stdbool.h"
#include "driverHWEEPROM.h"

extern bool driverSWStorageManagerConfigEmpty;
extern bool driverSWStorageManagerStateOfChargeEmpty;
extern uint16_t driverSWStorageManagerConfigStructSize;
extern uint16_t driverSWStorageManagerStateOfChargeStructSize;

typedef enum {
	STORAGE_CONFIG = 0,
	STORAGE_STATEOFCHARGE,
} StorageLocationTypedef;

void driverSWStorageManagerInit(void);
bool driverSWStorageManagerStoreConfigStruct(void *configStruct, StorageLocationTypedef storageLocation);
bool driverSWStorageManagerGetConfigStruct(void *configStruct, StorageLocationTypedef storageLocation);
uint16_t driverSWStorageManagerGetOffsetFromLocation(StorageLocationTypedef storageLocation);
uint16_t driverSWStorageManagerGetStructSize(StorageLocationTypedef storageLocation);

#endif
