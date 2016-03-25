/* Includes ------------------------------------------------------------------*/
#include "main.h"
//主晶振7.3728M      未倍频

int16_t TempValue[8] = {0,0,0,0,0,0,0,0};
uint16_t 	DelayTime = 0;//延时计数值(ms)
uint8_t 	DelayTimeFlag = 0;//计时标志
#define DRIVE_TIME	1500		//激励时间配置
#define	RETRY	3		//采样重试次数配置

unsigned int  SysTick_Count;//定义SysTick计数器
Send_Buff_tagdef         Send_Buff;					//定义向主机发送数据缓存区
Receive_CfgDATA_tagdef   Receive_CfgDATA;
__IO unsigned char Slave_TxBufLen=sizeof(Send_Buff);//定义I2C发送数据长度	//
__IO unsigned char *Pr_Tx=NULL;						//定义I2C发送数据指针
__IO unsigned char I2C1_Buffer_Rx[64];  //定义I2C接收数据缓冲区

//定义数据组合长度，用于向TPA发送一包数据
//定义通道1、2、3、4增益
float temp_data;                  //定义温度缓存数据，用于暂存采集到的温度数据
unsigned char *data_p=NULL;
int data_len;
unsigned char TEMP_Flag=0x00;       //定义温度采样标志位，成功：0，失败：1
unsigned char Stress_Flag=0x00;     //定义频率采样标志位，成功：0，失败：1
unsigned char  SensorType_Flag=0x00;//定义温度传感器类型标志，模拟类：0，数字类：1
unsigned char Sample_Status=0x00;	//定义采样状态缓存，0：采样成功，1：温度采样失败，2：频率采样失败，3：温度、频率采样均失败		
float FreqValue;					//定义频率缓存数据，用于暂存采集到的频率数据             

__IO uint32_t TimingDelay = 0;;
/**********************************************************************************
* Function Name  : Delay
* 延时函数,通过对time6设定
* nTime:时间值(ms)
**********************************************************************************/
void Delay(__IO uint32_t nTime)
{ 
	TIM6_Reconfiguration(nTime);
	TimingDelay = 1;
	while(TimingDelay != 0)__WFI;//等待过程中进入sleep模式,下方指令在执行过程中容易进入
	//硬件异常模式
	//PWR_EnterSleepMode(PWR_Regulator_LowPower,PWR_SLEEPEntry_WFI);
	TIM6_Disable();
}


/*******************************************************************************
* Function Name  : GPIO_PinReverse
* Description    : GPIO取反操作
* Input          : GPIOx，GPIO_PIN_x
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_PinReverse(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    GPIOx->ODR ^= GPIO_Pin;
}

/*******************************************************************************
* Function Name  : IWDG_INIT
* Description    : 
* Input          : FeedTime 喂狗时间 ms
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_INIT(uint32_t FeedTime)
{
  /* Enable the LSI oscillator ************************************************/
  RCC_LSICmd(ENABLE);
  
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET){}
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: LSI/256 */
  //37k/256=144.53125Hz=6.9ms
  IWDG_SetPrescaler(IWDG_Prescaler_256);

  IWDG_SetReload(FeedTime/7);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

/*******************************************************************************
* Function Name  : Open_Temp_Channel
* Description    : 打开对应通道的温度采集
* Input          : 通道参数1、2、3、4
* Output         : None
* Return         : None
*******************************************************************************/
void Open_Temp_Channel(unsigned char Channel)
{
	Channel -= 1;
	if(Channel & 0x01)TEMP_P0_H;
	else			  TEMP_P0_L;
	
	if(Channel & 0x02)TEMP_P1_H;
	else 			  TEMP_P1_L;
	
	if(Channel & 0x04)TEMP_P2_H;
	else			  TEMP_P2_L;	
}

/*******************************************************************************
* Function Name  : OpenSampleLed
* Description    : 指示对应通道振弦采样，该通道激励时，灯亮，采样完成，关闭，，轮询完，关闭所有灯
* Input          : 通道号，0,1,2,3,4
* Output         : None
* Return         : None
*******************************************************************************/
void OpenSampleLed(unsigned char ch)
{
	LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;
	switch(ch)
	{
		case 1:
			LED1_ON;
			break;
		case 2:
			LED2_ON;
			break;
		case 3:
			LED3_ON;
			break;
		case 4:
			LED4_ON;
			break;
		default:
			break;		
	}
}

/*******************************************************************************
* Function Name  : Open_FCTRL_Chansel
* Description    : 打开对应通道的FCTRL或Chansel
* Input          : 通道参数0、1、2、3、4，0表示关闭所有
* Output         : None
* Return         : None
*******************************************************************************/
void Open_FCTRL_Chansel(unsigned char F_Ch,unsigned char C_Ch)//参数01234
{
	FCTR1_L;FCTR2_L;FCTR3_L;FCTR4_L;            //关闭FCTR
	CHANSEL1_H;CHANSEL2_H;CHANSEL3_H;CHANSEL4_H;//关闭CHANSEL
	switch(F_Ch)
	{
		case 1:
			FCTR1_H;
			break;
		case 2:
			FCTR2_H;
			break;
		case 3:
			FCTR3_H;
			break;
		case 4:
			FCTR4_H;
			break;
		default:
			break;		
	}
	switch(C_Ch)
	{
		case 1:
			CHANSEL1_L;
			break;
		case 2:
			CHANSEL2_L;
			break;
		case 3:
			CHANSEL3_L;
			break;
		case 4:
			CHANSEL4_L;
			break;
		default:
			break;	
	}
}

/*******************************************************************************
* Function Name  : sample_stress_channel
* Description    : 首先对弦进行激励，然后采集对应通道的应变振弦数据
* Input          : 通道参数1、2、3、4，
* Output         : None
* Return         : None
*******************************************************************************/
static uint8_t db;
uint16_t err_cnt;
char sample_stress_channel(unsigned char channel)//参数1234
{
	float ival;
	float freq32;
	uint16_t i;
#ifdef	Debug_EN
	printf("频率采样通道：%d\r\n",channel);
#endif
	/*********************激励源处理******************************/
	if(DelayTime < DRIVE_TIME)Delay(DRIVE_TIME - DelayTime);//剩余激励时间进入低功耗
	DelayTime = 0;//清零,下次激励全时长
	DelayTimeFlag = 0;//开始计数
	PWM_Disable();//关闭pwm
	
	/*********************激励传感器******************************/
	Open_FCTRL_Chansel(channel,0);//拉高对应通道FCTRL
	Delay(10);	//10ms 等待稳定时间
	Open_FCTRL_Chansel(0,channel);//关闭激励开关，并打开Chansel
	Delay(50);	//50ms 等待稳定时间
	
	/*********************数据捕获******************************/
	TIM2_Enable();             //打开捕获
	while(!CaptureFinishFlag);//等待捕获结束
	CaptureFinishFlag = 0;
	OpenSampleLed(0);
	/*********************处理第一路数据******************************/   
	if(channel)
	{
		for( i=0; i<DATALEN; i++)  //处理数据
		{
			if(databuf2[i] > databuf2[i+1]){databuf2[i+1] = databuf2[i]+1;}//过滤错误数据
			databuf2[i] = databuf2[i+1]-databuf2[i]; //得到每个脉宽所用时钟数量
      //printf("buf[%d]= %d\r\n",i,databuf2[i]);
		}
		calc_itval_r2(databuf2, DATALEN, &ival, &db);
#ifdef	Debug_EN
		printf("Chan %d before_filter:k= %f db= %d freq= %.3f\r\n",channel, ival, db, (float) SystemCoreClock / ival);
#endif
		err_cnt = interval_filter(databuf2, DATALEN, &ival, &db);
#ifdef	Debug_EN
		printf("err_cnt= %d\r\n",err_cnt);
#endif
		if(!ival || !db)
			for( i=0; i<DATALEN; i++)printf("databuf1[%d] = %d\r\n", i, databuf2[i]);

		freq32 = ((float)SystemCoreClock )/ ival;
#ifdef	Debug_EN
		printf("Chan %d after_filter:k= %f db= %d err_cnt= %d freq= %.3f\r\n",channel, ival, db,err_cnt, freq32 );
#endif
		FreqValue=freq32;
		if((db < 35) || (err_cnt > 10))return ERROR;//测量失败则返回错误		
		return SUCCESS;//返回成功
	}
return ERROR;
}

/*******************************************************************************
* Function Name  : get_temp_analog
* Description    : 应变模拟温度信号采集，通过AD采集电桥电压，进而计算出热电阻对应的温度数据
	$T = 1/(A + B\ln{R_{temp}} + C(\ln{R_{temp}})3) - 273$

	where A=1.4051 x 10^(-3) when temperature is between -50C and 50C, B=2.369 x 10^(-4), C=1.019 x 10^(-7).

	The board measures the voltage output (at the port PADC1) of the above circuit for the temperature. R21, R23 and R26 are set with 4K ohm, R25 is set for 80 ohm and R22 is set for 0 ohm. The TEMP_PORT is connected to $R_{temp}$. The formula is the following:

	$R_{temp} = \dfrac{R_1R_2V_r + R_12V_{out}}{(R_1+R_2)V_r - R_1V_{out}}$

	Vr is the reference voltage, it is set to 1024( 3V where also used to connect to the ADC, which returns 1024 in digital format), 
	R1 is 4K ohm and R2 is 80 ohm
	Vout is the reading from the data packet

	Please refer to the following link for constructed excel sheet \\testbed.eenics.in\vmPublic_0__rw\Documents\传感器和节点电路\应变\应变传感器温度电阻计算.xlsx.
	http://www.sciweavers.org/free-online-latex-equation-editor   此网页可转换上述公式 有两个公式
* Input          : 通道参数1、2、3、4，
* Output         : None
* Return         : None
*******************************************************************************/
#define TEM_R1  30000
#define TEM_R2  80.6
#define VREF    65535
#define Temp_BUF_SIZE	5	//溫度採樣緩存
int32_t get_temp_analog(char i,unsigned char Channel)//参数1234
{
		double adval,rtemp,temp,logrtemp;
		double temp_sum;
		int j;
		static unsigned short int temp_buf[Temp_BUF_SIZE];

		//切换为模拟采集，判断数据，否则进入数字采集，并判断数据，否则返回一个很大的负数。
		
		TEMP_Switch_A;             //切换为模拟温度传感器
		AD7798_Init();             //初始化温度采集模块
		Open_Temp_Channel(Channel);//打开对应通道温度传感器的地
		Delay(30);				   //延时保证数据稳定
		temp_sum = 0;
		AD7798_ContinuousReadData(i,temp_buf,Temp_BUF_SIZE); 
		for(j = 0;j < Temp_BUF_SIZE;j++)
		{
			adval = temp_buf[j];
			rtemp = (TEM_R1 * TEM_R2 * VREF + TEM_R1* TEM_R1 * adval) / ((TEM_R1 + TEM_R2) * VREF - TEM_R1 * adval) ;
			logrtemp = log(rtemp);
			temp =  1 / (0.0014051 + 0.0002369 * log(rtemp) + 0.0000001019 * logrtemp * logrtemp * logrtemp) - 273;
			temp_sum += temp;
		}
		temp_data = temp_sum/Temp_BUF_SIZE ;	
#ifdef	Debug_EN
		printf("模拟温度%d:%f\r\n", Channel,temp_data);
#endif
     return SUCCESS;
}

/*******************************************************************************
* Function Name  : get_temp_ds18b20
* Description    : 获取ds18b20温度传感器的数据，
* Input          : 通道参数1、2、3、4，
* Output         : None
* Return         : None
*******************************************************************************/
int32_t get_temp_ds18b20(unsigned char Channel)//参数1234
{
	TEMP_Switch_D;//切换为数字温度
	Open_Temp_Channel(Channel);	
	Delay(30);					 //延时30ms，保证数据稳定
	temp_data=DS18B20_TEMP();
#ifdef	Debug_EN	
	printf("数字温度%f\r\n",temp_data);
#endif
	
	return SUCCESS;
}

/*******************************************************************************
* Function Name  : get_stress
* Description    : 振弦信号采集单元
* Input          : 通道参数1、2、3、4，
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t get_stress(unsigned char Channel)//参数1234
{
	char RetryVal;
	/***********再执行激励采样**********************/
	for(RetryVal = 0;RetryVal < RETRY;RetryVal++)//测量失败则重试
	{		
		//Delay(5);//延时等待电源稳定
		OpenSampleLed(Channel);
		PWM_Enable();//使能pwm输出
		DelayTimeFlag = 1;//开始计数
#ifdef	Debug_EN							
		printf("第%d次采样\r\n",RetryVal+1);
#endif
		if(sample_stress_channel(Channel) == SUCCESS)break;//应变信号采集并输出-->TPX	
		else if(RetryVal == 2)
		{
			Stress_Flag=0x01;
#ifdef	Debug_EN
			printf("      通道%d      振弦采样失败！\r\n",Channel);//重试次数用完仍失败则赋值0
#endif			
		}
	}
	PWM_Disable();

  return SUCCESS;
}

/*******************************************************************************
* Function Name  : DataPrepare
* Description    : 数据准备函数，用于将采集过程中的数据转存到发送缓冲区中
* Input          : 通道参数1、2、3、4
* Output         : None
* Return         : None
*******************************************************************************/
void DataPrepare(unsigned char Channel)//参数1234
{
		Send_Buff.Sensor_Data[Channel-1].Sample_Status                    = 0x00;
		Send_Buff.Frame_Length                                            = sizeof(Send_Buff);  //计算需要发送帧数据的长度
	    Send_Buff.Sensor_Data[Channel-1].Sensor_Type                      = SensorType_Flag;
		if(TEMP_Flag)  Send_Buff.Sensor_Data[Channel-1].Sample_Status    |= 0x01;
	    if(Stress_Flag)Send_Buff.Sensor_Data[Channel-1].Sample_Status    |= 0x02;
		Send_Buff.Sensor_Data[Channel-1].Freq_Value                       = FreqValue;
		Send_Buff.Sensor_Data[Channel-1].DB_Value                         = db;
		Send_Buff.Sensor_Data[Channel-1].Err_Value                        = err_cnt;
		Send_Buff.Sensor_Data[Channel-1].Temp_Value                       = temp_data;
		
		TEMP_Flag=0x00;
		Stress_Flag=0x00;
		SensorType_Flag=0x00;
}

/*******************************************************************************
* Function Name  : Instruction_Check
* Description    : 指令检查函数，用于检查主机发送的数据时候有效
* Input          : None
* Output         : None
* Return         : 成功：1，错误：0
*******************************************************************************/
unsigned char Instruction_Check(void)
{
	Receive_CfgDATA.SampleType= I2C1_Buffer_Rx[1];
	Receive_CfgDATA.Ch         = I2C1_Buffer_Rx[2];
	Receive_CfgDATA.SensorType= I2C1_Buffer_Rx[3];
	
	if(Receive_CfgDATA.SampleType==0)return SUCCESS;//如果是模式0，直接返回成功
	else if( (Receive_CfgDATA.SampleType<=2)  && (Receive_CfgDATA.Ch&0x0f) )//否则检查第三位
		return SUCCESS;
	else
	{
		Send_Buff.Command_Status=0x01;
		return ERROR;
	}		
}

/*******************************************************************************
* Function Name  : Auto_Smaple
* Description    : 自动采样模式，该模式下，从机自动识别四个通道传感器安装及类型
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Auto_Smaple(void)
{
	unsigned char i;
	for(i=1;i<5;i++)
	{		
		TEMP_Flag =0;
		FreqValue =0;
		temp_data =0;
		(void)get_temp_analog(0,i);	//执行模拟温度采样，先执行模拟采样，成功该位置0  
		if(temp_data<TEMPSENSOR_LIMIT)				//失败执行数字采样，成功该位置1
		{	
			(void)get_temp_ds18b20(i);//再执行数字温度采样
			 SensorType_Flag = 0x01;        
		}
		if(temp_data<TEMPSENSOR_LIMIT)
		{
			TEMP_Flag=0x01;//标记温度采样失败//都失败
#ifdef	Debug_EN
			printf("          通道%d温度采样失败\r\n",i);
#endif
		}
		/***********再执行激励采样**********************/
		if(!TEMP_Flag)
			{
				(void)(get_stress(i));
				DataPrepare(i);
			}
	}
}

/*******************************************************************************
* Function Name  : Half_Auto_Sample
* Description    : 半自动采样模式，该模式下，从机自动根据通道使能来自动判定传感器类型
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Half_Auto_Sample(void)
{
	unsigned char i;
	unsigned char DATA_tmp=0x01;
	for(i=1;i<5;i++)
	{
		if(Receive_CfgDATA.Ch & DATA_tmp)
		{
			(void)get_temp_analog(0,i);	//执行模拟温度采样，先执行模拟采样，成功该位置0  
			if(temp_data<TEMPSENSOR_LIMIT)				//失败执行数字采样，成功该位置1
			{	
				(void)get_temp_ds18b20(i);//再执行数字温度采样
				SensorType_Flag = 0x01;        
			}
			if(temp_data<TEMPSENSOR_LIMIT)
			{
				TEMP_Flag=0x01;//标记温度采样失败//都失败
#ifdef	Debug_EN
				printf("          通道%d温度采样失败\r\n",i);
#endif
			}
			/***********再执行激励采样**********************/
			(void)(get_stress(i));
			DataPrepare(i);	
		}
		DATA_tmp=DATA_tmp<<1;
	}
	DATA_tmp=0x01;
}

/*******************************************************************************
* Function Name  : Manual_Sample
* Description    : 手动采样模式，该模式下，从机自动根据通道使能、及传感器类型来采样
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Manual_Sample(void)
{
	unsigned char i;
	unsigned char DATA_tmp=0x01;
	for(i=1;i<5;i++)
	{
		if(Receive_CfgDATA.Ch & DATA_tmp)
		{
			if(Receive_CfgDATA.SensorType & DATA_tmp)
			{
				(void)get_temp_ds18b20(i);//再执行数字温度采样//执行数字采样
				SensorType_Flag = 0x01; 
			}
			else
				(void)get_temp_analog(0,i);//执行模拟采样
			if(temp_data<TEMPSENSOR_LIMIT)
			{
				TEMP_Flag=0x01;//标记温度采样失败//都失败
#ifdef	Debug_EN
				printf("          通道%d温度采样失败\r\n",i);
#endif
			}
			
		/***********再执行激励采样**********************/	
		(void)(get_stress(i));	
		DataPrepare(i);	
		}			
		DATA_tmp=DATA_tmp<<1;	//	
	}
	DATA_tmp=0x01;
}

/*******************************************************************************
* Function Name  : Process
* Description    : 采样进程函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Process(void)
{
	if(Receive_CfgDATA.SampleType==0)      Auto_Smaple();            //自动模式（每个通道自动判定是否有传感器、传感器类型）
			
	else if(Receive_CfgDATA.SampleType==1) Half_Auto_Sample();       //半自动模式（按选择的通道判别传感器类型）
			
	else				                   Manual_Sample();          //手动模式（按选择的通道、传感器类型采样）
}

/*******************************************************************************
* Function Name  : 主函数
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
	/******************外设配置***********************/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//中断优先级分组
	if (SysTick_Config(SystemCoreClock / 1000))while(1);	
	TIM6_Configuration();     //TIM6  延时
	NVIC_TIM6_Configuration();//TIM6中断配置
	GPIO_Configuration();     //GPIO
	I2C1_Init();
#ifdef  Debug_EN
	UART3_Configuration(115200);//UART3  调试打印
#endif
	SPI1_Configuration();
	TIM2_Configuration();
	PWM_Configuration(6000,50);
	
#ifdef	Debug_EN
	printf("Hardware had ready!\r\n");
#endif
  //初始化状态数据
	Send_Buff.Sensor_Data[0].Sample_Status                    = 0x03;
	Send_Buff.Sensor_Data[1].Sample_Status                    = 0x03;
	Send_Buff.Sensor_Data[2].Sample_Status                    = 0x03;
	Send_Buff.Sensor_Data[3].Sample_Status                    = 0x03;
	
  while(!I2C_Received_Flag);      //等待主机发送采样指令
	(void)Instruction_Check();		//指令检查函数
//	Receive_CfgDATA.SampleType= 2;
//	Receive_CfgDATA.Ch        = 0x0a;
//	Receive_CfgDATA.SensorType= 2;
	
	Process();	//采样任务
	EXIT_OUT_H;	//中断置位
	OpenSampleLed(0);
	while(1);				
}


