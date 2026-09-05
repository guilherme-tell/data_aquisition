#include "adc.h"


#define GPIOAEN		(1U<<0)
#define ADC1EN		(1U<<8)
#define CR2_ADCON	(1U<<0)
#define CR2_CONT	(1U<<1)
#define CR2_SWSTART	(1U<<30)
#define SR_EOC		(1U<<1)

void pa1_adc_init(void)
{
	//Configura o pino GPIO do ADC

	//Habilita o acesso da GPIOA ao clock no seu barramento
	RCC->AHB1ENR |= GPIOAEN;

	//Define modo analógico ao PA1
	GPIOA->MODER |= (1U<<2);
	GPIOA->MODER |= (1U<<3);

	//Configura o modulo do ADC
	//Habilita o acesso do modulo do ADC ao clock
	RCC->APB2ENR |= ADC1EN;

	//Define o start da sequencia de conversao
	ADC1->SQR3 = (1U<<0);

	//Define o comprimento da sequencia de conversao
	ADC1->SQR1 = 0x00;			/*conversao unica*/

	//Habilita o modulo ADC
	ADC1->CR2 |= CR2_ADCON;


}

void start_conversion(void)
{
	//Habilita conversao continua
	ADC1->CR2 |= CR2_CONT;

	//Inicia a conversao no ADC
	ADC1->CR2 |= CR2_SWSTART;
}

uint32_t adc_read(void)
{
	//Aguarda ate a conversao estar completa
	while(!(ADC1->SR & SR_EOC)){}

	//Le os resultados
	return (ADC1->DR);
}
