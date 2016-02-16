/**
  ******************************************************************************
  * @file    Project/STM32L1xx_StdPeriph_Templates/stm32l1xx_it.c 
  * @author  MCD Application Team
  * @version V1.2.0
  * @date    16-May-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//#include "stm32l1xx_it.h"
#include "main.h"
//#include "uart.h"
//#include "spi.h"

__IO unsigned char Tx_p=0,Rx_p=0;
__IO unsigned char i=0;
__IO unsigned int Event=0x00;
__IO unsigned int UNused_DATA;

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  if(CoreDebug->DHCSR & 1)
	{
		__breakpoint(0);
	}
	while (1)
  {
		
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

//外部中断0，，对应通道4
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
		
    /* Clear the EXTI line 0 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}

//外部中断1，，对应通道3
void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
		
    /* Clear the EXTI line 1 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

//外部中断2，，对应通道2
void EXTI2_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {
		
    /* Clear the EXTI line 2 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

//外部中断10，，对应通道1
void EXTI15_10_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line10) != RESET)
  {
		
    /* Clear the EXTI line 10 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line10);
  }
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	if(DelayTimeFlag)DelayTime++;
		SysTick_Count++;
		
}

void USART1_IRQHandler(void)
{
	unsigned char 	Uart_Get_Data;	//串口1接收的数据
	UART1_RBUF_ST *p = &uart1_rbuf;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		Uart_Get_Data = USART_ReceiveData(USART1);
		if((p->in - p->out)<UART1_RBUF_SIZE)
		{
			p->buf [p->in & (UART1_RBUF_SIZE-1)] = Uart_Get_Data;	
			p->in++;
			Uart1Flag = 1;
		}
	}
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : 串口3中断服务函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
	unsigned char 	Uart_Get_Data;	//串口2接收一个字节的数据
	UART3_RBUF_ST *p = &uart3_rbuf;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		Uart_Get_Data = USART_ReceiveData(USART3);
		if((p->in - p->out)<UART3_RBUF_SIZE)
		{
			p->buf [p->in & (UART3_RBUF_SIZE-1)] = Uart_Get_Data;	
			p->in++;
			Uart3Flag = 1;
		}
	}
}

/**
  * @brief  This function handles I2C1 Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C1_EV_IRQHandler(void)
{
	Event=I2C_GetLastEvent(I2C1);	//获取最新的Event事情
    switch (Event)
    {	
    /* 从机发送数据 Slave Transmitter ---------------------------------------------------*/
		case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:	//EV1 从机，地址匹配
			Tx_p=0;											//清零计数器
			Pr_Tx = &(Send_Buff.Frame_Length);			//指针指向结构体第一个元素
		break;	  
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:           		//EV3 数据已经传输，读取数据到缓冲区
	case I2C_EVENT_SLAVE_BYTE_TRANSMITTING:                 /* EV3 数据正在传输*/ 
		if(Tx_p<Slave_TxBufLen)								//如果发送任务未完成
		{
			//I2C_SendData(I2C1,I2C1_Buffer_Rx[Tx_p++]);		//继续发送字节
			I2C_SendData(I2C1,*(Pr_Tx++));		//继续发送字节
		}
		else
		{
			Tx_p=0;								//清零计数器
			Pr_Tx = &(Send_Buff.Frame_Length);
			I2C_Cmd(I2C1, ENABLE);							//置位PE
		}			
		break;
		
				
    /* 从机接收数据  Slave Receiver ------------------------------------------------------*/
	case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:     /* EV1 */
		UNused_DATA=I2C1->SR1;
		UNused_DATA=I2C1->SR2;
		Rx_p=0;
	break;
	case I2C_EVENT_SLAVE_BYTE_RECEIVED:				//EV2 如果字节接收已完成
    case I2C_EVENT_SLAVE_BYTE_RECEIVED | 0x0004:    /* EV2  BTF置位*/
				 I2C1_Buffer_Rx[Rx_p++] = I2C1->DR;	//获取数据寄存器中的数据
        break;

    case I2C_EVENT_SLAVE_STOP_DETECTED:             		/* EV4 */
				I2C_GetFlagStatus(I2C1,I2C_FLAG_STOPF);		//获取STOPF位状态
				I2C_Cmd(I2C1, ENABLE);						//重新使能I2C1，PE=1
				I2C_Received_Flag=1;
#ifdef Debug_EN 
				if(Rx_p>=I2C1_Buffer_Rx[0])
				{
					for(Rx_p=0;Rx_p<I2C1_Buffer_Rx[0];Rx_p++)
					{
						printf("Rx:0x%x\r\n",I2C1_Buffer_Rx[Rx_p]);
						
					}
					Rx_p=0;
					printf("\r\n");
				}
#endif
    break;
	case I2C_END_RECEIVED:					//在main.h中自定义状态，适用于主机直接发送，不管DSR寄存器
		I2C1_Buffer_Rx[Rx_p++] = I2C1->DR;	//传输后读一次DR寄存器，并获取最后一个数据，完成硬件清零中断标志，最后I2C进入EV4
	break;
    default:			
    break;	
    }			
}

void I2C1_ER_IRQHandler(void)
{
	/* Check on I2C1 SMBALERT flag and clear it */
#ifdef Debug_EN
	Event=I2C_GetLastEvent(I2C1);			
	//printf("Error Event:   0x%x\r\n",Event);
	if((Event & 0xff00) != 0x00)
	{
		I2C1->SR1 &= 0x00ff;
	}
#else
  if( ((I2C1->SR1) & 0xff00) != 0x00 )//如果进入错误中断
	{
		I2C1->SR1 &= 0x00ff;		//清除标志位
	}
#endif
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : 定时器2中断服务函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler(void)
{
	int iGetCapture;		
	if (TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);
		iGetCapture = TIM_GetCapture3(TIM2);
        
		if(CaptureFlag2 < DATALEN+1)
		{
			databuf2[CaptureFlag2] = (CaptureNumber<<16) |  iGetCapture; 
			CaptureFlag2 ++;
		}
	}
	
	if(CaptureFlag2 > DATALEN)//缓存全满则完成采样
    {
      CaptureFinishFlag = 1;//标记捕获已完成
			CaptureNumber = 0;
      CaptureFlag2 = 0;
      TIM2_Disable();//关闭采样中断
    }
	
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清楚标志位
		if(CaptureFlag2 )CaptureNumber++; //如果已经记录第一次的计数值，那么加一
	}
}

void TIM6_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update); //清除标志位
		if(TimingDelay)TimingDelay--;
	}
}






/******************************************************************************/
/*                 STM32L1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l1xx_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
