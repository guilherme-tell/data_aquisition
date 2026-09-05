#include "fpu.h"


void fpu_enable(void)
{
	/*Habilita a Unidade de Ponto Flutuante*/
	SCB->CPACR |= (1U<<20);
	SCB->CPACR |= (1U<<21);
	SCB->CPACR |= (1U<<22);
	SCB->CPACR |= (1U<<23);

}
