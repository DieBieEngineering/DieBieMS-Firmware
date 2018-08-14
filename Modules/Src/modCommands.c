#include "modCommands.h"

// Private variables
static uint8_t modCommandsSendBuffer[PACKET_MAX_PL_LEN];
static void(*modCommandsSendFunction)(unsigned char *data, unsigned int len) = 0;
bool jumpBootloaderTrue;
modConfigGeneralConfigStructTypedef *modCommandsGeneralConfig;
modConfigGeneralConfigStructTypedef *modCommandsToBeSendConfig;
modConfigGeneralConfigStructTypedef modCommandsConfigStorage;

void modCommandsInit(modConfigGeneralConfigStructTypedef *configPointer) {
	modCommandsGeneralConfig = configPointer;
	jumpBootloaderTrue = false;
}

void modCommandsSetSendFunction(void(*func)(unsigned char *data, unsigned int len)) {
	modCommandsSendFunction = func;
}

void modCommandsSendPacket(unsigned char *data, unsigned int len) {
	if (modCommandsSendFunction) {
		modCommandsSendFunction(data, len);
	}
}

void modCommandsProcessPacket(unsigned char *data, unsigned int len) {
	if (!len) {
		return;
	}

	COMM_PACKET_ID packet_id;
	int32_t ind = 0;
	uint16_t flash_res;
	uint32_t new_app_offset;
	uint32_t delayTick;

	packet_id = (COMM_PACKET_ID) data[0];
	data++;
	len--;

	switch (packet_id) {
		case COMM_FW_VERSION:
			ind = 0;
			modCommandsSendBuffer[ind++] = COMM_FW_VERSION;
			modCommandsSendBuffer[ind++] = FW_VERSION_MAJOR;
			modCommandsSendBuffer[ind++] = FW_VERSION_MINOR;
			strcpy((char*)(modCommandsSendBuffer + ind), HW_NAME);
			ind += strlen(HW_NAME) + 1;
			memcpy(modCommandsSendBuffer + ind, STM32_UUID_8, 12);
			ind += 12;

			modCommandsSendPacket(modCommandsSendBuffer, ind);
			break;
		case COMM_JUMP_TO_BOOTLOADER:
			jumpBootloaderTrue = true;
			delayTick = HAL_GetTick();
			break;
		case COMM_ERASE_NEW_APP:
			ind = 0;
			flash_res = modFlashEraseNewAppData(buffer_get_uint32(data, &ind));

			ind = 0;
			modCommandsSendBuffer[ind++] = COMM_ERASE_NEW_APP;
			modCommandsSendBuffer[ind++] = flash_res == HAL_OK ? true : false;
			modCommandsSendPacket(modCommandsSendBuffer, ind);
			break;
		case COMM_WRITE_NEW_APP_DATA:
			ind = 0;
			new_app_offset = buffer_get_uint32(data, &ind);
			flash_res = modFlashWriteNewAppData(new_app_offset, data + ind, len - ind);

			ind = 0;
			modCommandsSendBuffer[ind++] = COMM_WRITE_NEW_APP_DATA;
			modCommandsSendBuffer[ind++] = flash_res == HAL_OK ? 1 : 0;
			modCommandsSendPacket(modCommandsSendBuffer, ind);
			break;
		case COMM_GET_VALUES:
			break;
		case COMM_SET_MCCONF:
			ind = 0;
		  modCommandsGeneralConfig->noOfCells                      = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->batteryCapacity                = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellHardUnderVoltage           = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellHardOverVoltage            = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellLCSoftUnderVoltage         = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellHCSoftUnderVoltage         = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellSoftOverVoltage            = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellBalanceDifferenceThreshold = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellBalanceStart               = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellThrottleUpperStart         = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellThrottleLowerStart         = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellThrottleUpperMargin        = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->cellThrottleLowerMargin        = buffer_get_float32_auto(data,&ind);
		  modCommandsGeneralConfig->throttleChargeIncreaseRate     = buffer_get_uint8(data,&ind);
		  modCommandsGeneralConfig->throttleDisChargeIncreaseRate  = buffer_get_uint8(data,&ind);
		  modCommandsGeneralConfig->cellBalanceUpdateInterval      = buffer_get_uint32(data,&ind);
		  modCommandsGeneralConfig->maxSimultaneousDischargingCells = buffer_get_uint8(data,&ind);
		  modCommandsGeneralConfig->timeoutDischargeRetry          = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->hysteresisDischarge            = buffer_get_float32_auto(data,&ind);
		  modCommandsGeneralConfig->timeoutChargeRetry             = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->hysteresisCharge               = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->timeoutChargeCompleted         = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->timeoutChargingCompletedMinimalMismatch = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->maxMismatchThreshold           = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->chargerEnabledThreshold        = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->timeoutChargerDisconnected     = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->minimalPrechargePercentage     = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->timeoutLCPreCharge             = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->maxAllowedCurrent              = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->displayTimeoutBatteryDead      = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->displayTimeoutBatteryError     = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->displayTimeoutBatteryErrorPreCharge = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->displayTimeoutSplashScreen     = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->maxUnderAndOverVoltageErrorCount = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->notUsedCurrentThreshold        = buffer_get_float32_auto(data,&ind);
			modCommandsGeneralConfig->notUsedTimeout                 = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->stateOfChargeStoreInterval     = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->CANID                          = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->CANIDStyle                     = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->tempEnableMaskBMS              = buffer_get_uint16(data,&ind);
			modCommandsGeneralConfig->tempEnableMaskBattery          = buffer_get_uint16(data,&ind);
		  modCommandsGeneralConfig->LCUseDischarge                 = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->LCUsePrecharge                 = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->allowChargingDuringDischarge   = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->allowForceOn                   = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->pulseToggleButton              = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->togglePowerModeDirectHCDelay   = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->useCANSafetyInput              = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->useCANDelayedPowerDown         = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->HCUseRelay                     = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->HCUsePrecharge                 = buffer_get_uint8(data,&ind);
			modCommandsGeneralConfig->timeoutHCPreCharge             = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->timeoutHCPreChargeRetryInterval= buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->timeoutHCRelayOverlap          = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTCTopResistor[modConfigNTCGroupLTCExt]         = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTC25DegResistance[modConfigNTCGroupLTCExt]     = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTCBetaFactor[modConfigNTCGroupLTCExt]          = buffer_get_uint16(data,&ind);
			modCommandsGeneralConfig->NTCTopResistor[modConfigNTCGroupMasterPCB]      = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTC25DegResistance[modConfigNTCGroupMasterPCB]  = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTCBetaFactor[modConfigNTCGroupMasterPCB]       = buffer_get_uint16(data,&ind);
			modCommandsGeneralConfig->NTCTopResistor[modConfigNTCGroupHiAmpExt]       = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTC25DegResistance[modConfigNTCGroupHiAmpExt]   = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTCBetaFactor[modConfigNTCGroupHiAmpExt]        = buffer_get_uint16(data,&ind);
			modCommandsGeneralConfig->NTCTopResistor[modConfigNTCGroupHiAmpPCB]       = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTC25DegResistance[modConfigNTCGroupHiAmpPCB]   = buffer_get_uint32(data,&ind);
			modCommandsGeneralConfig->NTCBetaFactor[modConfigNTCGroupHiAmpPCB]        = buffer_get_uint16(data,&ind);
		
			ind = 0;
			modCommandsSendBuffer[ind++] = packet_id;
			modCommandsSendPacket(modCommandsSendBuffer, ind);
		
			break;
		case COMM_GET_MCCONF:
		case COMM_GET_MCCONF_DEFAULT:
      if(packet_id == COMM_GET_MCCONF_DEFAULT){
				modConfigLoadDefaultConfig(&modCommandsConfigStorage);
				modCommandsToBeSendConfig = &modCommandsConfigStorage;
			}else{
				modCommandsToBeSendConfig = modCommandsGeneralConfig;
			}
		
      ind = 0;
		  modCommandsSendBuffer[ind++] = packet_id;
		  
		  buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->noOfCells,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->batteryCapacity,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellHardUnderVoltage,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellHardOverVoltage,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellLCSoftUnderVoltage,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellHCSoftUnderVoltage,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellSoftOverVoltage,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellBalanceDifferenceThreshold,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellBalanceStart,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellThrottleUpperStart,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellThrottleLowerStart,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellThrottleUpperMargin,&ind);
		  buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->cellThrottleLowerMargin,&ind);
		  buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->throttleChargeIncreaseRate,&ind);
		  buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->throttleDisChargeIncreaseRate,&ind);
		  buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->cellBalanceUpdateInterval,&ind);
		  buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->maxSimultaneousDischargingCells,&ind);
		  buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutDischargeRetry,&ind);
			buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->hysteresisDischarge,&ind);
		  buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutChargeRetry,&ind);
			buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->hysteresisCharge,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutChargeCompleted,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutChargingCompletedMinimalMismatch,&ind);
			buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->maxMismatchThreshold,&ind);
			buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->chargerEnabledThreshold,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutChargerDisconnected,&ind);
			buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->minimalPrechargePercentage,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutLCPreCharge,&ind);
			buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->maxAllowedCurrent,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->displayTimeoutBatteryDead,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->displayTimeoutBatteryError,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->displayTimeoutBatteryErrorPreCharge,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->displayTimeoutSplashScreen,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->maxUnderAndOverVoltageErrorCount,&ind);
			buffer_append_float32_auto(modCommandsSendBuffer,modCommandsToBeSendConfig->notUsedCurrentThreshold,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->notUsedTimeout,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->stateOfChargeStoreInterval,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->CANID,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->CANIDStyle,&ind);
			buffer_append_uint16(modCommandsSendBuffer,modCommandsToBeSendConfig->tempEnableMaskBMS,&ind);
			buffer_append_uint16(modCommandsSendBuffer,modCommandsToBeSendConfig->tempEnableMaskBattery,&ind);
		  buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->LCUseDischarge,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->LCUsePrecharge,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->allowChargingDuringDischarge,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->allowForceOn,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->pulseToggleButton,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->togglePowerModeDirectHCDelay,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->useCANSafetyInput,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->useCANDelayedPowerDown,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->HCUseRelay,&ind);
			buffer_append_uint8(modCommandsSendBuffer,modCommandsToBeSendConfig->HCUsePrecharge,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutHCPreCharge,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutHCPreChargeRetryInterval,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->timeoutHCRelayOverlap,&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCTopResistor[modConfigNTCGroupLTCExt],&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTC25DegResistance[modConfigNTCGroupLTCExt],&ind);
			buffer_append_uint16(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCBetaFactor[modConfigNTCGroupLTCExt],&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCTopResistor[modConfigNTCGroupMasterPCB],&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTC25DegResistance[modConfigNTCGroupMasterPCB],&ind);
			buffer_append_uint16(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCBetaFactor[modConfigNTCGroupMasterPCB],&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCTopResistor[modConfigNTCGroupHiAmpExt],&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTC25DegResistance[modConfigNTCGroupHiAmpExt],&ind);
			buffer_append_uint16(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCBetaFactor[modConfigNTCGroupHiAmpExt],&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCTopResistor[modConfigNTCGroupHiAmpPCB],&ind);
			buffer_append_uint32(modCommandsSendBuffer,modCommandsToBeSendConfig->NTC25DegResistance[modConfigNTCGroupHiAmpPCB],&ind);
			buffer_append_uint16(modCommandsSendBuffer,modCommandsToBeSendConfig->NTCBetaFactor[modConfigNTCGroupHiAmpPCB],&ind);

		  modCommandsSendPacket(modCommandsSendBuffer, ind);
		
			break;
		case COMM_TERMINAL_CMD:
		  data[len] = '\0';
		  terminal_process_string((char*)data);
			break;
		case COMM_REBOOT:
			// Lock the system and enter an infinite loop. The watchdog will reboot.
			break;
		case COMM_ALIVE:
			break;
		case COMM_FORWARD_CAN:
			modCANSendBuffer(data[0], data + 1, len - 1, false);
			break;
		case COMM_STORE_BMS_CONF:
			modConfigStoreConfig();
		
			ind = 0;
			modCommandsSendBuffer[ind++] = packet_id;
			modCommandsSendPacket(modCommandsSendBuffer, ind);
			break;
		default:
			break;
	}
	
	if(modDelayTick1ms(&delayTick,1000) && jumpBootloaderTrue)
		modFlashJumpToBootloader();
}

void modCommandsPrintf(const char* format, ...) {
	va_list arg;
	va_start (arg, format);
	int len;
	static char print_buffer[255];

	print_buffer[0] = COMM_PRINT;
	len = vsnprintf(print_buffer+1, 254, format, arg);
	va_end (arg);

	if(len > 0) {
		modCommandsSendPacket((unsigned char*)print_buffer, (len<254)? len+1: 255);
	}
}
