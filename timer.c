#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_gpio.h"

extern void CAN_TxMsg(void);
extern int GEAR_MODE(int,int);
extern int GEAR_DIRECTION(int,int);
extern int distance_covered(void);
extern int speed(void);
extern void config_pins(void);
extern int read_temp(void);
extern void latch_status(void);

extern int s1,s2,s3,s4;
extern  unsigned long int RpmCnt;
 
TIM_HandleTypeDef htim3;

uint8_t msec = 0, sec = 0;



void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    __HAL_RCC_TIM3_CLK_ENABLE();
  if(htim_base->Instance==TIM3)
  {
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
 
	}
}

void TIM3_init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 499;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 3999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim3); 
	
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
  
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
}



void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim3);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	msec++;
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	if (htim-> Instance == TIM3)
	{
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
		GEAR_MODE(s1,s2);
		GEAR_DIRECTION(s3,s4);
		speed();
		RpmCnt=0;
		distance_covered();
		read_temp();
		latch_status();
		CAN_TxMsg();
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
	}
	if(msec == 4)
	{
		sec++;
		msec = 0;
	}
}
