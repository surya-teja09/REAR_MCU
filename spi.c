#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"

extern SPI_HandleTypeDef hspi1;


int read_temp(void);
extern void SPI_Init(void);

uint8_t data[2];
uint16_t temp,temp1;
float cel;
extern uint8_t msg[5];

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi1)
{
  GPIO_InitTypeDef Spi_InitConfig;
  if(hspi1->Instance == SPI1)
  {
    
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    Spi_InitConfig.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    Spi_InitConfig.Mode = GPIO_MODE_AF_PP;
    Spi_InitConfig.Speed = GPIO_SPEED_FAST;
    Spi_InitConfig.Alternate = GPIO_AF5_SPI1;
    Spi_InitConfig.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &Spi_InitConfig);
    
    HAL_NVIC_SetPriority(SPI1_IRQn,15,0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
  }
}


void SPI_Init(void)
{
	
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&hspi1); 
  
}


void SPI_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&hspi1);
}


int read_temp(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
	HAL_SPI_Receive(&hspi1,data,2,10);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
	temp = (data[0] << 8)|(data[1]);
	if (temp & 0x4)
		while(1); 
	temp1 =(temp >> 3);
	cel  = (temp*0.25);
	cel = cel/10;
	msg[0] = cel;
	return msg[0];
}
