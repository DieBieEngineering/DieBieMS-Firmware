#include "driverSWUART2.h"

libRingBufferTypedef *driverSWUART2OutputBuffer;
libRingBufferTypedef *driverSWUART2InputBuffer;

FILE driverSWUART2IOStream = {0};

void driverSWUART2Init(void) {
	driverSWUART2OutputBuffer = libRingBufferNew(sizeof(uint8_t),RINGBUFFERSIZE);	// Make new output buffer
	driverSWUART2InputBuffer = libRingBufferNew(sizeof(uint8_t),RINGBUFFERSIZE);	// Make new input buffer
	
	driverSWUART2IOStream.outputFunctionPointer = driverSWUART2PutCharInBuffer;		// couple output function pointer to stream output
	
	if(!driverSWUART2OutputBuffer)																								// Check if buffer pointer is generated
		while(true);																																// Out of memory error
	
	driverHWUART2Init(&driverSWUART2CharReceivedCallback);													// Initialize serial port and pass function that should be called when a byte is received
};

void driverSWUART2CharReceivedCallback(UART_HandleTypeDef *huart) {
	uint8_t receivedChar = driverHWUART2GetChar();
};

/*
void driverSWUART2Printf(const char *format, ...) {
	va_list args;
  va_start(args, format);
  vfprintf(&driverSWUART2IOStream,format, args);
  va_end (args);
};
*/

char driverSWUART2PutCharInBuffer(char character, FILE *stream) {
	if(!driverSWUART2OutputBuffer->isFull(driverSWUART2OutputBuffer))
		driverSWUART2OutputBuffer->add(driverSWUART2OutputBuffer,&character);
	return 0;
};

void driverSWUART2Task(void) {
	char outputChar;
	
	if(!driverSWUART2OutputBuffer->isEmpty(driverSWUART2OutputBuffer)){
		driverSWUART2OutputBuffer->pull(driverSWUART2OutputBuffer,&outputChar);
		driverHWUART2SendChar(outputChar);
	}
};
