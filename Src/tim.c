#include "tim.h"

#define TIM2EN     (1U << 0)
#define CR1_CEN   (1U << 0)
#define DIER_UIE  (1U << 0)

void tim2_interrupt(void)
{
    /* Habilita acesso de clock a TIM2 */
    RCC->APB1ENR |= TIM2EN;

    /* Desabilita o timer previo a configuracao*/
    TIM2->CR1 &= ~CR1_CEN;

    /* 3. Define prescaler e auto-reload */
    TIM2->PSC = 1171.875-1;   // 76.8 MHz / 1000 = 76.8 kHz
    TIM2->ARR = 10-1;      // 76.8 kHz / 10 = 7.68 kHz

    /* Limpa flags de contagem e interrupcao */
    TIM2->CNT = 0U;
    TIM2->SR  = 0U;

    /* Habilita atualizacao de interrupcao*/
    TIM2->DIER |= DIER_UIE;

    /* Habilita a interrupcao oriunda de TIM2 na NVIC */
    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);

    /* Habilita o Timer */
    TIM2->CR1 |= ~CR1_CEN;
}

void tim2_stop(void)
{
		TIM2->DIER = ~DIER_UIE;
		TIM2->CR1 = ~CR1_CEN;
}
