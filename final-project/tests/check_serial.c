#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "serial.h"

#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define SR REG(0x40004800)
#define DR REG(0x40004804)
void initUSART3(void) {}

int main(void)
{
    assert(mmap((void *)0x40004000, 0x2000, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) != MAP_FAILED);
    initSerial();
    char c = 'q';
    SR = 0; DR = 123;
    assert(readSerialCharacter(&c) == 0 && c == 'q' && DR == 123);
    SR = 0x20; DR = 'h';
    assert(readSerialCharacter(&c) == 1 && c == 'h');
    assert(beginSerialMessage());
    writeSerialText("Value: "); writeSerialNumber(0);
    writeSerialText("/"); writeSerialNumber(UINT32_MAX); writeSerialText("\r\n");
    assert(beginSerialMessage() == 0);
    SR = 0; DR = 987;
    for(unsigned int i = 0; i < 1000; i++) serviceSerialOutput();
    assert(DR == 987 && beginSerialMessage() == 0);
    const char *expected = "Value: 0/4294967295\r\n";
    SR = 0x80;
    for(size_t i = 0; i < strlen(expected); i++)
    {
        DR = 0xFFFFFFFF; serviceSerialOutput(); assert(DR == (uint32_t)expected[i]);
    }
    DR = 0xFFFFFFFF; serviceSerialOutput(); assert(DR == 0xFFFFFFFF);
    assert(beginSerialMessage());
    char large[400]; memset(large, 'a', 399); large[399] = 0;
    writeSerialText(large); writeSerialNumber(UINT32_MAX);
    for(unsigned int i = 0; i < SERIAL_MESSAGE_SIZE; i++)
    {
        DR = 0xFFFFFFFF; serviceSerialOutput(); assert(DR == 'a');
    }
    DR = 0xFFFFFFFF; serviceSerialOutput(); assert(DR == 0xFFFFFFFF);
    assert(beginSerialMessage()); writeSerialText("next");
    for(unsigned int i = 0; i < 4; i++)
    {
        serviceSerialOutput(); assert(DR == (uint32_t)"next"[i]);
    }
    puts("PASS: non-blocking serial RX/TX, one character per call, decimal formatting and bounded message storage.");
}
