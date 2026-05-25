
#include "stm32f4xx.h"

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  while(1);
}



