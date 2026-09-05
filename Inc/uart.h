#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include "stm32f446xx.h"

void uart2_tx_init(void);
void uart2_write(int ch);

#endif
