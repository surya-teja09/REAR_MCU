#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

void config_pins(void);
void SystemClock_Config_HSE(void);
int GEAR_MODE(int,int);
int GEAR_DIRECTION(int,int);
int distance_covered(void);
int speed(void);
uint32_t pclk1;
void CAN1_Init(void);
void SPI_Init(void);

void CAN_Filter_Config(void);

extern void TIM3_init(void);

CAN_HandleTypeDef hcan1;
SPI_HandleTypeDef hspi1;
GPIO_InitTypeDef PinConfig;

extern TIM_HandleTypeDef htim3;

int s1,s2,s3,s4;
uint8_t msg[5];
unsigned long int RpmCnt;
int kmph;
double odo;
float low_gear_ratio = 13.9;
float high_gear_ratio = 7.25;
float gear_ratio;
	
int main(void)
{
	HAL_Init();
	
	config_pins();
	
	SystemClock_Config_HSE();
	
	CAN1_Init();
	
	CAN_Filter_Config();
	
	HAL_CAN_Start(&hcan1);
	
	SPI_Init();
	
	HAL_CAN_ActivateNotification(&hcan1,CAN_IT_TX_MAILBOX_EMPTY|CAN_IT_RX_FIFO0_MSG_PENDING|CAN_IT_BUSOFF); //initiates callback function whenever the interrupts are raised
	
	TIM3_init();
	
	 HAL_TIM_Base_Start_IT(&htim3);
	
	pclk1 = HAL_RCC_GetPCLK1Freq();
	while(1)
	{ 
		s1 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0);
		s2 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1);
		s3 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2);
		s4 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3);
	}
}

void config_pins(void)
{
	
	__HAL_RCC_GPIOC_CLK_ENABLE(); //enabling rcc clock for port a gpio pins
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	PinConfig.Pin = GPIO_PIN_0 |  GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;      
	PinConfig.Mode = GPIO_MODE_INPUT;
	PinConfig.Speed = GPIO_SPEED_FAST;
	PinConfig.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC,&PinConfig);
	
	/*enabling interrupts for the GPIO's and SysTick Timer*/
	
	HAL_NVIC_SetPriority(EXTI0_IRQn,0,1);   
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	
	HAL_NVIC_SetPriority(EXTI1_IRQn,0,1);   
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	
	HAL_NVIC_SetPriority(EXTI2_IRQn,0,1);   
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	
	HAL_NVIC_SetPriority(EXTI3_IRQn,0,1);   
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()); 
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	HAL_NVIC_SetPriority(SysTick_IRQn,0,0);

  PinConfig.Pin = GPIO_PIN_7;      
	PinConfig.Mode = GPIO_MODE_IT_RISING;
	PinConfig.Speed = GPIO_SPEED_FAST;
	PinConfig.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB,&PinConfig);
	
	HAL_NVIC_SetPriority(EXTI9_5_IRQn,0,1);   
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	PinConfig.Pin = GPIO_PIN_5;      
	PinConfig.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(GPIOA,&PinConfig);
	
	
	PinConfig.Pin = GPIO_PIN_10;      
	PinConfig.Mode = GPIO_MODE_OUTPUT_PP; 
	HAL_GPIO_Init(GPIOB,&PinConfig);

}

/*CallBack function for gpio status*/

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	if(GPIO_Pin == GPIO_PIN_7)
		RpmCnt++;
	#if 0
	else if(GPIO_Pin == GPIO_PIN_0)
	{
		s1 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0); 
		s1 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0); 
	}
	
	else if(GPIO_Pin == GPIO_PIN_1) 
	{
		s2 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1);
		s2 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1);
	}
	
	else if(GPIO_Pin == GPIO_PIN_2) 
	{
		s3 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2);
		s3 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2);
	}
	
	else if(GPIO_Pin == GPIO_PIN_3) 
	{
		s4 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3);
		s4 = HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3);
	}
	
	else
		__NOP();	
	#endif
}

/*System clock configuration*/

void SystemClock_Config_HSE()
{
	RCC_OscInitTypeDef Osc_Init; //typedef for oscillator initialisation
	RCC_ClkInitTypeDef Clk_Init; //type def for clock initialisation
	
 	Osc_Init.OscillatorType = RCC_OSCILLATORTYPE_HSE;  //HSE(high speed external oscillator is used)
	Osc_Init.HSEState = RCC_HSE_BYPASS; //oscillator is connected from external st link circuit so bypass is used
	
	HAL_RCC_OscConfig(&Osc_Init); //HAL function to initialise the oscillator configuration
	
	Clk_Init.ClockType =  RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2; //selecting clock type 
	Clk_Init.SYSCLKSource = RCC_SYSCLKSOURCE_HSE; //clock source from High speed external oscillator
	Clk_Init.AHBCLKDivider = RCC_SYSCLK_DIV1; //System clock is divided by 1 prescalar
	Clk_Init.APB1CLKDivider = RCC_HCLK_DIV1;  //High speed clock is divided by 1 prescalar for peripheral bus 1
	Clk_Init.APB2CLKDivider = RCC_HCLK_DIV1;  //High speed clock is divided by 1 prescalar for peripheral bus 2
	
	
	/*flash latency is to read the data from flash memory our clock freqency is 8MHz so the flash latency is 0*/
	
	HAL_RCC_ClockConfig(&Clk_Init,FLASH_ACR_LATENCY_0WS); //HAL function to configure the clock initialisation 
}

	/*SysTick handler function */

void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}


/*IRQ handler for gpio pin 0*/

void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}


/*IRQ handler for gpio pin 1*/

void EXTI1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}


/*IRQ handler for gpio pin 2*/

void EXTI2_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}


/*IRQ handler for gpio pin 3*/

void EXTI3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}


void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}


/*for gear mode*/

int GEAR_MODE(int a,int b)
{
	if(a==0 && b==1) 		//gear low mode
	{
		gear_ratio = low_gear_ratio;
		return msg[1] = 1; 
	}  
	else if(a==1 && b==0) //gear auto
	{	
		gear_ratio = high_gear_ratio;
		return msg[1] = 2;
	}
	else if(a==1 && b==1)		//gear high 
	{	
		gear_ratio = high_gear_ratio;
		return msg[1] = 3;
	}
	else 
		return msg[1] = 0;
}


/*for gear direction*/

int GEAR_DIRECTION(int a,int b)
{
	if(a==0 && b==1)   //gear neutral`
		return msg[2] = 1; 
	else if(a==1 && b==0) //gear reverse
		return msg[2] = 2;
	else if(a==1 && b==1) //gear drive 
		return msg[2] = 3;
	else 
		return msg[2] = 0;
}

int speed(void)
{
  	kmph = RpmCnt*15*50/653/gear_ratio*4;
    return msg[3] = kmph;
}

int distance_covered(void)
{
	odo += (double) kmph/3600/4;
	if (odo >= 1) {
    msg[4] = (int) odo;
    odo = 0;
	}
	else {
		msg[4] = 0;
	}
	return msg[4];
}

