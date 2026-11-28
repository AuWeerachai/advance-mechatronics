#include "serial.h"
#include "stm32f446xx_mort.h"
#include "stm32f446xx_usart3_mort.h"

// Use the existing USART3 pins and initialization. Runtime transfers do not wait.
#define USART3_SR_REGISTER (USART3_BASE_MORT + 0x00)
#define USART3_DR_REGISTER (USART3_BASE_MORT + 0x04)

#define USART3_SR_RX_READY_MASK ((uint32_t)0x20)
#define USART3_SR_TX_READY_MASK ((uint32_t)0x80)

static char message[SERIAL_MESSAGE_SIZE];
static uint32_t messageLength;
static uint32_t nextCharacter;

void initSerial(void)
{
    initUSART3();
    messageLength = 0;
    nextCharacter = 0;
}

uint32_t readSerialCharacter(char * character)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)USART3_SR_REGISTER;
    if((*reg_pointer & USART3_SR_RX_READY_MASK) == 0)
    {
        return 0;
    }
    reg_pointer = (volatile uint32_t *)USART3_DR_REGISTER;
    *character = (char)(*reg_pointer & (uint32_t)0xFF);
    return 1;
}

uint32_t beginSerialMessage(void)
{
    if(nextCharacter != messageLength)
    {
        return 0;
    }
    messageLength = 0;
    nextCharacter = 0;
    return 1;
}

void writeSerialText(const char * text)
{
    // Copy at most one fixed-size message; no waiting for the UART here.
    while(*text != '\0' && messageLength < SERIAL_MESSAGE_SIZE)
    {
        message[messageLength] = *text;
        messageLength = messageLength + 1;
        text = text + 1;
    }
}

void writeSerialNumber(uint32_t number)
{
    char digits[10];
    uint32_t count;

    count = 0;
    do
    {
        digits[count] = '0' + number % 10;
        count = count + 1;
        number = number / 10;
    } while(number != 0);

    while(count != 0 && messageLength < SERIAL_MESSAGE_SIZE)
    {
        count = count - 1;
        message[messageLength] = digits[count];
        messageLength = messageLength + 1;
    }
}

void serviceSerialOutput(void)
{
    volatile uint32_t * reg_pointer;

    if(nextCharacter == messageLength)
    {
        return;
    }
    reg_pointer = (volatile uint32_t *)USART3_SR_REGISTER;
    if((*reg_pointer & USART3_SR_TX_READY_MASK) == 0)
    {
        return; // Try again next main-loop iteration.
    }
    reg_pointer = (volatile uint32_t *)USART3_DR_REGISTER;
    *reg_pointer = (uint32_t)(unsigned char)message[nextCharacter];
    nextCharacter = nextCharacter + 1;
}
