#ifndef __TIM_H__
#define __TIM_H__


#include "stm32f446xx.h"

#define SR_UIF	(1U<<0)

void tim2_interrupt(void);
void tim2_stop(void);

#endif
