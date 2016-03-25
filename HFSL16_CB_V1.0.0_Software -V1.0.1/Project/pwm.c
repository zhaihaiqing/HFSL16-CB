/* Includes ------------------------------------------------------------------*/
#include "main.h"



/**********************************************************************************
* Function Name  : PWM_Configuration
* PWM初始化
* PwmFreq:频率值
* DutyCycles:占空比
**********************************************************************************/
void PWM_Configuration(unsigned int PwmFreq,unsigned char DutyCycles)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t PrescalerValue = 0;
	
 	 /* TIM4 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
 	/*--------------------------------- GPIO Configuration -------------------------*/
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;//变压器电源
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);
		/* -----------------------------------------------------------------------
		The TIM4 is running at 12 KHz: TIM4 Frequency = TIM4 counter clock/(ARR + 1)
                                                  = 32 MHz / 666 = 12 KHz
  ----------------------------------------------------------------------- */
	/* Compute the prescaler value */
	//PrescalerValue = (uint16_t) (SystemCoreClock / (200000 + 1));
	PrescalerValue = 0;
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = SystemCoreClock / PwmFreq - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	/* PWM Mode configuration: Channe2 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = (DutyCycles * (SystemCoreClock / PwmFreq - 1)) / 100;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
}
/**********************************************************************************
* Function Name  : PWM_Enable
* PWM启动
**********************************************************************************/
void PWM_Enable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*--------------------------------- GPIO Configuration -------------------------*/
	/* GPIOB Configuration: Pin 7 */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
	
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	 /* TIM4 enable counter */
	TIM_Cmd(TIM4, ENABLE);
}
/**********************************************************************************
* Function Name  : PWM_Disable
* PWM关闭
**********************************************************************************/
void PWM_Disable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*--------------------------------- GPIO Configuration -------------------------*/
	/* GPIOB Configuration: Pin 7 */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_7);
	
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Disable);
	 /* TIM3 enable counter */
	TIM_Cmd(TIM4, DISABLE);
}
/********************************************************************
激励电压输出控制

	频率			时间			电压
	5kHz			1.4S			92.5V
	6kHz			2s				122.5V	
	7kHz			2.5s			137.5V
	8kHz			2.9s			140V
	9kHz			3.5s			140V
********************************************************************/
//void Stim(void){
// 	PWM_Enable();
//	Delay(2000);	//1600ms 激励时间
// 	PWM_Disable();
//		FCTR1_H;
//		FCTR2_H;
//		FCTR3_H;

//	Delay(10);	//10ms 等待稳定时间
//	FCTR1_L;
//	FCTR2_L;
//	FCTR3_L;
// 	Delay(50);	//160ms 等待稳定时间
// 
//}


