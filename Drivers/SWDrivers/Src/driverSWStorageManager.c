#include "driverSWStorageManager.h"

uint16_t *driverHWEEPROMVirtAddVarTab;

bool driverSWStorageManagerConfigEmpty = false;
bool driverSWStorageManagerStateOfChargeEmpty = false;
uint16_t driverSWStorageManagerDataSize = 0;
uint16_t driverSWStorageManagerConfigStructSize = 0;
uint16_t driverSWStorageManagerStateOfChargeStructSize = 0;

void driverSWStorageManagerInit(void) {
	driverSWStorageManagerDataSize = driverSWStorageManagerConfigStructSize + driverSWStorageManagerStateOfChargeStructSize;																	// Calculate the total desired space
	
	// Generate EEPROM enviroment
	driverHWEEPROMVirtAddVarTab = malloc(driverSWStorageManagerDataSize);																					// Make room for the addres array
	HAL_FLASH_Unlock();																																														// Unlock FLASH to allow EEPROM lib to write
	
	if(driverHWEEPROMInit(driverSWStorageManagerDataSize) != HAL_OK)																							// Init EEPROM and tell EEPROM manager the disered size
		while(true);																																																// Something went wrong in the init
	
	for(uint16_t addresPointer=0 ; addresPointer<driverSWStorageManagerDataSize ; addresPointer++)
		driverHWEEPROMVirtAddVarTab[addresPointer] = addresPointer;																									// Generate addres array
	
	// Check EEPROM Contents
	uint16_t readVariable;																																												// Define variable to write EEPROM contents to if any
	if(driverHWEEPROMReadVariable(driverHWEEPROMVirtAddVarTab[0], &readVariable) == 1) {													// Try to read from first memory location
		driverSWStorageManagerConfigEmpty = true;
		driverSWStorageManagerStateOfChargeEmpty = true;
	}
};

bool driverSWStorageManagerStoreConfigStruct(void *configStruct) {
	uint16_t *dataPointer = (uint16_t*)configStruct;																																// Trick to convert struct to a 16 bit pointer.
	
	for(uint16_t addresPointer=0 ; addresPointer<driverSWStorageManagerConfigStructSize ; addresPointer++)					// Pass trough all adresses
		driverHWEEPROMWriteVariable(driverHWEEPROMVirtAddVarTab[addresPointer],dataPointer[addresPointer]);						// Store data in EEPROM
	
	return false;
};

bool driverSWStorageManagerGetConfigStruct(void *configStruct) {
	uint16_t *dataPointer = (uint16_t*)configStruct;																																// Trick to convert struct to a 16 bit pointer.
	
	for(uint16_t addresPointer=0 ; addresPointer<driverSWStorageManagerConfigStructSize ; addresPointer++)
		driverHWEEPROMReadVariable(driverHWEEPROMVirtAddVarTab[addresPointer],&dataPointer[addresPointer]);
	
	return false;
};


