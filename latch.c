#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

void latch_status(void);

extern uint8_t msg[6];
uint8_t l1;

void latch_status()
{
	l1=HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_12);
	if(l1 == 1)
	 msg[5] = 0; //latch is closed
	else if(l1 == 0)
	 msg[5] = 2; //latch is open
	else
		__NOP();
}
