#include "uart.h"


#define GPIOAEN			(1U<<0)
#define UART2EN			(1U<<17)
#define CR1_TE			(1U<<3)
#define CR1_UE			(1U<<13)

#define SR_TXE			(1U<<7)


#define SYS_FREQ		180000000
#define APB1_CLK		(SYS_FREQ/4)

#define UART_BAUDRATE	115200

static void uart2_set_baudrate(uint32_t periph_clk, uint32_t baudrate);

void uart2_write(int ch);

int __io_putchar(int ch)
{
	uart2_write(ch);

	return ch;
}

void uart2_tx_init(void)
{
	/*****Configura o pino GPIO da UART******/
	/*Habilita acesso de clock a GPIOA*/
	RCC->AHB1ENR |= GPIOAEN;

	/*Define o modo de PA2 para Alternate Function */
	GPIOA->MODER &=~(1U<<4);
	GPIOA->MODER |=(1U<<5);

	/*Define PA2 Alternate Function para UART_TX(AF07)*/
	GPIOA->AFR[0] |=(1U<<8);
	GPIOA->AFR[0] |=(1U<<9);
	GPIOA->AFR[0] |=(1U<<10);
	GPIOA->AFR[0] &=~(1U<<11);

	/*****Configura UART  ******/
	/*Habilita acesso de clock a UART2*/
	RCC->APB1ENR |=UART2EN;

	/*Configura o baudrate*/
	uart2_set_baudrate(APB1_CLK,UART_BAUDRATE);

	/*Configura a direcao de transferencia*/
	USART2->CR1 = CR1_TE;

	/*Habilita o modulo UART*/
	USART2->CR1 |= CR1_UE;

}

void uart2_write(int ch)
{
	/*Garante que o registrador de dados transmitidos esta vazio*/
	   while(!(USART2->SR & SR_TXE)){}

	/*Escreve no registrador de transmissao*/
	   USART2->DR = (ch &0xFF);
}

static uint16_t compute_uart_bd( uint32_t periph_clk, uint32_t baudrate)
{
	return ((periph_clk + (baudrate/2U))/baudrate);
}

static void uart2_set_baudrate(uint32_t periph_clk, uint32_t baudrate)
{
	USART2->BRR = compute_uart_bd(periph_clk,baudrate);
}
