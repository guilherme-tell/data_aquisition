#include "gpio.h"
#include "stm32f446xx.h"

void gpio_interrupt(void)
{
	__disable_irq();
	RCC->AHB1ENR |= 1;				//Habilita o acesso de clock a PORT A -> Output event (Pino D7)
	RCC->AHB1ENR |= 4;				//Habilita o acesso de clock a PORT C -> PUSH BUTTON
	RCC->APB2ENR |= 0x4000;			//Habilita o acesso de clock a SYSCFG


	GPIOA->MODER		|= 0x10000;		//Define PA8 as output mode Define PA8 em modo de output
	SYSCFG->EXTICR[3]	|= 0x0020;		//Seleciona PORT C par EXTI13

	EXTI->IMR	|= 0x2000;			//Unmask de EXTI13
	EXTI->FTSR	|= 0x2000;			//Seleciona o trigger em borda de descida

	NVIC_EnableIRQ(EXTI15_10_IRQn);

	__enable_irq();
}
