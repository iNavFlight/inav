
#include "platform.h"
#include "rcc.h"

#define RCC_BIT_CMD(ptr, mask, state)       do { if (state != DISABLE) { ptr |= mask; } else { ptr &= ~mask; } } while(0)


void RCC_ClockCmd(rccPeriphTag_t periphTag, FunctionalState NewState)
{
    int tag = periphTag >> 5;
    uint32_t mask = 1 << (periphTag & 0x1f);

    switch (tag) {
#if defined(STM32F3)
    case RCC_AHB:
        RCC_BIT_CMD(RCC->AHBENR, mask, NewState);
        break;
#endif
    case RCC_APB2:
        RCC_BIT_CMD(RCC->APB2ENR, mask, NewState);
        break;
    case RCC_APB1:
        RCC_BIT_CMD(RCC->APB1ENR, mask, NewState);
        break;
#if defined(STM32F4) || defined(STM32F7)
    case RCC_AHB1:
        RCC_BIT_CMD(RCC->AHB1ENR, mask, NewState);
        break;
#endif
    }
}

void RCC_ResetCmd(rccPeriphTag_t periphTag, FunctionalState NewState)
{
    int tag = periphTag >> 5;
    uint32_t mask = 1 << (periphTag & 0x1f);

    switch (tag) {
#if defined(STM32F3)
    case RCC_AHB:
        RCC_BIT_CMD(RCC->AHBRSTR, mask, NewState);
        break;
#endif
    case RCC_APB2:
        RCC_BIT_CMD(RCC->APB2RSTR, mask, NewState);
        break;
    case RCC_APB1:
        RCC_BIT_CMD(RCC->APB1RSTR, mask, NewState);
        break;
#if defined(STM32F4) || defined(STM32F7)
    case RCC_AHB1:
        RCC_BIT_CMD(RCC->AHB1RSTR, mask, NewState);
        break;
#endif
    }
}
