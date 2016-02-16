/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include "time.h"

/*******************************************************************************
* Function Name  : TIM2_Configuration
* Description    : 定时器2初始化设置
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//void TIM2_Configuration(void)
//{
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	TIM_DeInit(TIM2);					//复位TIM2
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//	
//	//中断优先级NVIC设置
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;  //从优先级3级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
//	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
//	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
//  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM2中断,允许更新中断	
//	
//	TIM_InternalClockConfig(TIM2);
//	TIM_TimeBaseStructure.TIM_Period = 400;//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
//  TIM_TimeBaseStructure.TIM_Prescaler = 36863;//设置用来作为TIMx时钟频率除数的预分频值,32M/(32000-1)=1KhZ
//  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分割:TDTS = Tck_tim
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
//	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);//根据指定的参数初始化TIMx的时间基数单位
//	TIM_Cmd(TIM2, ENABLE);  //使能TIMx	
//}

//void TIM3_Configuration(void)
//{
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	TIM_DeInit(TIM3);					//复位TIM3
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//	
//	//中断优先级NVIC设置
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
//	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级2级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
//	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
//	TIM_ClearFlag(TIM3,TIM_FLAG_Update);
//  TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE ); //使能指定的TIM3中断,允许更新中断	
//	
//	//   29.4912M/(2950-1)=10K
//	TIM_InternalClockConfig(TIM3);
//	TIM_TimeBaseStructure.TIM_Period = 100;//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
//  TIM_TimeBaseStructure.TIM_Prescaler = 2950;//设置用来作为TIMx时钟频率除数的预分频值,主时钟不倍频不分频，=800hZ
//  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分割:TDTS = Tck_tim
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);//根据指定的参数初始化TIMx的时间基数单位
//	TIM_Cmd(TIM3, DISABLE);  //使能TIMx	
//}

/**********************************************************************************
* Function Name  : TIM6_Configuration
* TIM6初始化
**********************************************************************************/
void TIM6_Configuration(void)
{
	/* TIM6 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
}
/**********************************************************************************
* Function Name  : TIM6_Reconfiguration
* TIM6重新配置
* nTime:延时值
**********************************************************************************/
void TIM6_Reconfiguration(__IO uint32_t nTime)
{ 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = nTime;
  TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	TIM6->CNT = 0;
		/* TIM IT enable */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}
/**********************************************************************************
* Function Name  : TIM6_Disable
* TIM6关闭
**********************************************************************************/
void TIM6_Disable(void)
{
  /* TIM IT disable */
	TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
  /* TIM6 disable counter */
  TIM_Cmd(TIM6, DISABLE);
}

/*******************************************************************************
* Function Name  : NVIC_TIM6_Configuration
* 捕获中断 初始化
*******************************************************************************/
void NVIC_TIM6_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the TIM6 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn; //捕获中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占式
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}






