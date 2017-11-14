#include "driverSWUART2.h"

libRingBufferTypedef *driverSWUART2OutputBuffer;

FILE driverSWUART2IOStream = {0};

void driverSWUART2Init(uint32_t baudRate) {
	driverSWUART2OutputBuffer = libRingBufferNew(sizeof(uint8_t),RINGBUFFERSIZE);	// Make new output buffer	
	driverSWUART2IOStream.outputFunctionPointer = driverSWUART2PutCharInOutputBuffer;// couple output function pointer to stream output
	
	if(!driverSWUART2OutputBuffer)																								// Check if buffer pointer is generated
		while(true);																																// Out of memory error
	
	driverHWUART2Init(baudRate);																									// Initialize serial port and pass function that should be called when a byte is received
};

char driverSWUART2PutCharInOutputBuffer(char character, FILE *stream) {
	// TODO: If buffer is full, first send a character out, then place new char. This should however never happen
	if(!driverSWUART2OutputBuffer->isFull(driverSWUART2OutputBuffer))
		driverSWUART2OutputBuffer->add(driverSWUART2OutputBuffer,&character);
	return 0;
};

bool driverSWUART2Task(void) {
	char outputChar;
	char inputChar;
	
	if(!driverSWUART2OutputBuffer->isEmpty(driverSWUART2OutputBuffer)){						// Check if there is data in the ouput buffer
		driverSWUART2OutputBuffer->pull(driverSWUART2OutputBuffer,&outputChar);			// Pull the data from ouput buffer
		driverHWUART2SendChar(outputChar);																					// And send it to the uart
	}
	
	if(driverHWUART2GetChar(&inputChar))																					// Loop received data back to output
		driverSWUART2PutCharInOutputBuffer(inputChar,&driverSWUART2IOStream);
	
	return !driverSWUART2OutputBuffer->isEmpty(driverSWUART2OutputBuffer);
};
