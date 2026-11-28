#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define SERIAL_MESSAGE_SIZE 384U

void initSerial(void);
uint32_t readSerialCharacter(char * character);
// Returns 0 if the previous message is still being sent; never waits.
uint32_t beginSerialMessage(void);
void writeSerialText(const char * text);
void writeSerialNumber(uint32_t number);
void serviceSerialOutput(void);

#endif
