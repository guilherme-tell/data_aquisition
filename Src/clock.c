#include "clock.h"

#define PLL_M	4
#define PLL_N	180
#define PLL_P	2
#define PLL_Q	2

/*
 * System clock source = HSE
 * SYSCLK 			   = 180MHz
 * HCLK				   = 180MHz
 * AHB Prescaler       = 1
 * APB1 Prescaler      = 4
 * APB2 Prescaler      = 2
 * HSE                 = 8MHz*/

/*
 * System clock source = HSE
 * SYSCLK 			   = 153.6 MHz
 * HCLK				   = 153.6 MHz
 * AHB Prescaler       = 1
 * APB1 Prescaler      = 4
 * APB2 Prescaler      = 2
 * HSE                 = 8 MHz*/


void clock_config(void)
{
    /* Habilita HSE */
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0) {}

    /* Configura a PLL */
    RCC->PLLCFGR =
          PLL_M
        | (PLL_N << 6)
        | (((PLL_P >> 1) - 1) << 16)
        | (PLL_Q << 24)
        | RCC_PLLCFGR_PLLSRC_HSE;

    /* Habilita PLL */
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0) {}

    /* Configura a FLASH */
    FLASH->ACR =
          FLASH_ACR_LATENCY_5WS
        | FLASH_ACR_PRFTEN
        | FLASH_ACR_ICEN
        | FLASH_ACR_DCEN;

    /* Prescalers */
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

    /* Seleciona a PLL como SYSCLK */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}
}
