#include "modUART.h"

uint32_t consoleStatusLastTick;

void modUARTInit(void) {	
	libPacketInit(modUARTSendPacket, modUARTProcessPacket, PACKET_HANDLER_UART);
	
	driverSWUART2Init(115200);																								// Configure the UART driver
};

void modUARTTask(void) {
	char inputChar;
	
	while(driverSWUART2Task()){};
		
	if(driverHWUART2GetChar(&inputChar))
		libPacketProcessByte(inputChar,PACKET_HANDLER_UART);
};

void modUARTSendPacketWrapper(unsigned char *data, unsigned int len) {
	libPacketSendPacket(data, len, PACKET_HANDLER_UART);
}

void modUARTProcessPacket(unsigned char *data, unsigned int len) {
	modCommandsSetSendFunction(modUARTSendPacketWrapper);
	modCommandsProcessPacket(data, len);
}

void modUARTSendPacket(unsigned char *data, unsigned int len) {
	static uint8_t buffer[PACKET_MAX_PL_LEN + 5];
	memcpy(buffer, data, len);
	fwrite (buffer , sizeof(uint8_t), len, &driverSWUART2IOStream);
}

void modUARTQueMessage(modMessageMessageTypeDef messageType, const char* format, ...) {
	uint32_t timeStamp = HAL_GetTick();
	switch(messageType) {
		case MESSAGE_DEBUG:
			fprintf(&driverSWUART2IOStream,"[%10.03f] [DEBUG] ",((float)timeStamp)/1000);
			break;
		case MESSAGE_ERROR:
			fprintf(&driverSWUART2IOStream,"[%10.03f] [ERROR] ",((float)timeStamp)/1000);
			break;
		case MESSAGE_NORMAL:
			fprintf(&driverSWUART2IOStream,"[%10.03f] ",((float)timeStamp)/1000);
			break;
	};
	
	va_list argptr;
	va_start(argptr, format);
	vfprintf(&driverSWUART2IOStream, format, argptr);
	va_end(argptr);
	
	while(driverSWUART2Task());
};
