#include "stm32f4xx_hal_can.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

extern void CAN1_Init(void);
extern void CAN_Filter_Config(void);
void CAN_TxMsg(void);
extern void latch_status(void);

extern uint8_t msg[6];
extern uint8_t l1;

extern CAN_HandleTypeDef hcan1;
CAN_TxHeaderTypeDef TxHeader; // trasmission header typedef
CAN_RxHeaderTypeDef RxHeader;

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan1)
{
  GPIO_InitTypeDef Can_InitConfig;
  if(hcan1->Instance==CAN1)
  {
    
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    Can_InitConfig.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    Can_InitConfig.Mode = GPIO_MODE_AF_PP;
    Can_InitConfig.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    Can_InitConfig.Alternate = GPIO_AF9_CAN1;
    Can_InitConfig.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &Can_InitConfig);
    
    /* Enable Interrupt vector */
   //HAL_NVIC_SetPriority(CAN1_TX_IRQn,15,0);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn,15,0);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn,15,0);

    //HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  }
}


void CAN1_Init(void)
{
	hcan1.Instance = CAN1; //registers base address
	hcan1.Init.Mode = CAN_MODE_NORMAL; //using can in normal mode
	hcan1.Init.AutoBusOff = DISABLE;  
	hcan1.Init.AutoRetransmission = ENABLE; //retransmit the data when failed to transmit once
	hcan1.Init.AutoWakeUp = ENABLE; //wakeup from the sleep mode
	hcan1.Init.ReceiveFifoLocked = DISABLE; //to keep updating with latest data received always
	hcan1.Init.TimeTriggeredMode = DISABLE; // enable triggers when timers are set
	hcan1.Init.TransmitFifoPriority = DISABLE; //transmit according to the identifier values of the data frame in mailbox is disabled
	
	//CAN bit timings settings
	
	hcan1.Init.Prescaler=1; //precalar value 
	hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;  //syncronization jump width 
	hcan1.Init.TimeSeg1 = CAN_BS1_13TQ; //propagation segment + phase segment 1
	hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;  //phase segment 2
	
	HAL_CAN_Init(&hcan1); //HAL function to initialise CAN
}


/*CAN Filter configurations*/

void CAN_Filter_Config(void)
{
	CAN_FilterTypeDef can1_filter_init; // filter typedef

	can1_filter_init.FilterActivation = ENABLE; //activating filter
	can1_filter_init.FilterBank  = 0; // to choose 1 of the filter bank in 28 filter banks
	can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0; //fifo to be filtered
	can1_filter_init.FilterIdHigh = 0x0000; //filter id high
	can1_filter_init.FilterIdLow = 0x0000; //filter id low
	can1_filter_init.FilterMaskIdHigh = 0X0000; //filter mask id high
	can1_filter_init.FilterMaskIdLow = 0x0000;  //filter mask id low
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK; //choosing between identifier list mode or mask mode 
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT; // filter scale can be choosed as 16bit or 32bit

	HAL_CAN_ConfigFilter(&hcan1,&can1_filter_init); //HAL function to configure the filter 
}

/*CAN transmit callback function*/

void CAN_TxMsg(void)

{
	uint32_t TxMailbox; //transmision mailbox
	
	TxHeader.DLC = 6;       //data code length 
	TxHeader.ExtId = 0x5DC; //CAN frame identifier
	TxHeader.IDE = CAN_ID_EXT; //Extended CAN
	TxHeader.RTR = CAN_RTR_DATA; //remote transmission request  i.e., configuring to data
	HAL_CAN_AddTxMessage(&hcan1,&TxHeader,msg,&TxMailbox); // HAL function to transmit data outside 
}

/*CAN receive callback function*/

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  uint8_t rcvd_msg[5];
  
  HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, rcvd_msg);  //HAL function to receieve data from outside

	if(RxHeader.ExtId == 0x3ED && RxHeader.RTR == 1)
	{
		if (rcvd_msg[0] == 1)
		{
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);
		}
	}
}

/*CAN transmit IRQ Handler*/

/*void CAN1_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&hcan1);
}*/

/*CAN receive IRQ Handler*/

void CAN1_RX0_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan1);
}

void CAN1_SCE_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&hcan1);
}
