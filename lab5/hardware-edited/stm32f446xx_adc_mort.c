#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_rcc_mort.h"
#include "stm32f446xx_timer_mort.h"

//PART 1.1
#define ADC3_BASE_ADDRESS ((uint32_t)0x40012200)
#define ADC3_SR_REGISTER (ADC3_BASE_ADDRESS + 0x00)
#define ADC3_CR1_REGISTER (ADC3_BASE_ADDRESS + 0x04)
#define ADC3_CR2_REGISTER (ADC3_BASE_ADDRESS + 0x08)
#define ADC3_SMPR2_REGISTER (ADC3_BASE_ADDRESS + 0x10)
#define ADC3_SQR1_REGISTER (ADC3_BASE_ADDRESS + 0x2C)
#define ADC3_SQR3_REGISTER (ADC3_BASE_ADDRESS + 0x34)
#define ADC_COMMON_CCR_REGISTER ((uint32_t)0x40012304)

//PART 1.1
#define ADC_CCR_PRESCALER_MASK ((uint32_t)0x03 << 16)
#define ADC_CCR_PRESCALER_DIV4 ((uint32_t)0x01 << 16)
#define ADC_CR1_SCAN ((uint32_t)0x01 << 8)
#define ADC_CR2_CONTINUOUS ((uint32_t)0x01 << 1)
#define ADC_CR2_DMA ((uint32_t)0x01 << 8)
#define ADC_CR2_DMA_CONTINUE ((uint32_t)0x01 << 9)
#define ADC_CR2_EOC_EACH_CONVERSION ((uint32_t)0x01 << 10)
#define ADC_CR2_ENABLE ((uint32_t)0x01)
#define ADC_CR2_START ((uint32_t)0x01 << 30)

//PART 1.1
void initADC3(uint32_t numberOfChannels)
{
    volatile uint32_t * reg_pointer;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);


    //CR2
    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;
    *reg_pointer = 0;

    //CCR2
    reg_pointer = (volatile uint32_t *)ADC_COMMON_CCR_REGISTER;
    *reg_pointer = *reg_pointer & ~ADC_CCR_PRESCALER_MASK;
    *reg_pointer = *reg_pointer | ADC_CCR_PRESCALER_DIV4;

    //CR1
    reg_pointer = (volatile uint32_t *)ADC3_CR1_REGISTER;
    *reg_pointer = 0;
    if(numberOfChannels == 3)
    {
        *reg_pointer = *reg_pointer | ADC_CR1_SCAN;
    }

    //CR2
    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;
    *reg_pointer = *reg_pointer | ADC_CR2_CONTINUOUS;
    *reg_pointer = *reg_pointer | ADC_CR2_DMA;
    *reg_pointer = *reg_pointer | ADC_CR2_DMA_CONTINUE;
    *reg_pointer = *reg_pointer | ADC_CR2_EOC_EACH_CONVERSION;

    //SQR1
    reg_pointer = (volatile uint32_t *)ADC3_SQR1_REGISTER;
    *reg_pointer = 0;
    if(numberOfChannels == 3)
    {
        *reg_pointer = (uint32_t)2 << 20;
    }

    //SQR3
    reg_pointer = (volatile uint32_t *)ADC3_SQR3_REGISTER;
    *reg_pointer = (uint32_t)5;
    if(numberOfChannels == 3)
    {
        *reg_pointer = *reg_pointer | ((uint32_t)6 << 5);
        *reg_pointer = *reg_pointer | ((uint32_t)7 << 10);
    }

    //SMPR2
    reg_pointer = (volatile uint32_t *)ADC3_SMPR2_REGISTER;
    *reg_pointer = (uint32_t)7 << 15;
    if(numberOfChannels == 3)
    {
        *reg_pointer = *reg_pointer | ((uint32_t)7 << 18);
        *reg_pointer = *reg_pointer | ((uint32_t)7 << 21);
    }

    //SR
    reg_pointer = (volatile uint32_t *)ADC3_SR_REGISTER;
    *reg_pointer = 0;

    //CR2
    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;
    *reg_pointer = *reg_pointer | ADC_CR2_ENABLE;
    delayMicroseconds(10);
}

//PART 1.1
void startADC3(void)
{
    volatile uint32_t * reg_pointer;

    //CR2
    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;
    *reg_pointer = *reg_pointer | ADC_CR2_START;
}
