/* Includes ------------------------------------------------------------------*/
#include "main.h"
//������7.3728M      δ��Ƶ

int16_t TempValue[8] = {0,0,0,0,0,0,0,0};
uint16_t 	DelayTime = 0;//��ʱ����ֵ(ms)
uint8_t 	DelayTimeFlag = 0;//��ʱ��־
#define DRIVE_TIME	1500		//����ʱ������
#define	RETRY	3		//�������Դ�������

unsigned int  SysTick_Count;//����SysTick������
Send_Buff_tagdef         Send_Buff;					//�����������������ݻ�����
Receive_CfgDATA_tagdef   Receive_CfgDATA;
__IO unsigned char Slave_TxBufLen=sizeof(Send_Buff);//����I2C�������ݳ���	//
__IO unsigned char *Pr_Tx=NULL;						//����I2C��������ָ��
__IO unsigned char I2C1_Buffer_Rx[64];  //����I2C�������ݻ�����

//����������ϳ��ȣ�������TPA����һ������
//����ͨ��1��2��3��4����
float temp_data;                  //�����¶Ȼ������ݣ������ݴ�ɼ������¶�����
unsigned char *data_p=NULL;
int data_len;
unsigned char TEMP_Flag=0x00;       //�����¶Ȳ�����־λ���ɹ���0��ʧ�ܣ�1
unsigned char Stress_Flag=0x00;     //����Ƶ�ʲ�����־λ���ɹ���0��ʧ�ܣ�1
unsigned char  SensorType_Flag=0x00;//�����¶ȴ��������ͱ�־��ģ���ࣺ0�������ࣺ1
unsigned char Sample_Status=0x00;	//�������״̬���棬0�������ɹ���1���¶Ȳ���ʧ�ܣ�2��Ƶ�ʲ���ʧ�ܣ�3���¶ȡ�Ƶ�ʲ�����ʧ��		
float FreqValue;					//����Ƶ�ʻ������ݣ������ݴ�ɼ�����Ƶ������             

__IO uint32_t TimingDelay = 0;;
/**********************************************************************************
* Function Name  : Delay
* ��ʱ����,ͨ����time6�趨
* nTime:ʱ��ֵ(ms)
**********************************************************************************/
void Delay(__IO uint32_t nTime)
{ 
	TIM6_Reconfiguration(nTime);
	TimingDelay = 1;
	while(TimingDelay != 0)__WFI;//�ȴ������н���sleepģʽ,�·�ָ����ִ�й��������׽���
	//Ӳ���쳣ģʽ
	//PWR_EnterSleepMode(PWR_Regulator_LowPower,PWR_SLEEPEntry_WFI);
	TIM6_Disable();
}


/*******************************************************************************
* Function Name  : GPIO_PinReverse
* Description    : GPIOȡ������
* Input          : GPIOx��GPIO_PIN_x
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
* Input          : FeedTime ι��ʱ�� ms
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
* Description    : �򿪶�Ӧͨ�����¶Ȳɼ�
* Input          : ͨ������1��2��3��4
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
* Description    : ָʾ��Ӧͨ�����Ҳ�������ͨ������ʱ��������������ɣ��رգ�����ѯ�꣬�ر����е�
* Input          : ͨ���ţ�0,1,2,3,4
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
* Description    : �򿪶�Ӧͨ����FCTRL��Chansel
* Input          : ͨ������0��1��2��3��4��0��ʾ�ر�����
* Output         : None
* Return         : None
*******************************************************************************/
void Open_FCTRL_Chansel(unsigned char F_Ch,unsigned char C_Ch)//����01234
{
	FCTR1_L;FCTR2_L;FCTR3_L;FCTR4_L;            //�ر�FCTR
	CHANSEL1_H;CHANSEL2_H;CHANSEL3_H;CHANSEL4_H;//�ر�CHANSEL
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
* Description    : ���ȶ��ҽ��м�����Ȼ��ɼ���Ӧͨ����Ӧ����������
* Input          : ͨ������1��2��3��4��
* Output         : None
* Return         : None
*******************************************************************************/
static uint8_t db;
uint16_t err_cnt;
char sample_stress_channel(unsigned char channel)//����1234
{
	float ival;
	float freq32;
	uint16_t i;
#ifdef	Debug_EN
	printf("Ƶ�ʲ���ͨ����%d\r\n",channel);
#endif
	/*********************����Դ����******************************/
	if(DelayTime < DRIVE_TIME)Delay(DRIVE_TIME - DelayTime);//ʣ�༤��ʱ�����͹���
	DelayTime = 0;//����,�´μ���ȫʱ��
	DelayTimeFlag = 0;//��ʼ����
	PWM_Disable();//�ر�pwm
	
	/*********************����������******************************/
	Open_FCTRL_Chansel(channel,0);//���߶�Ӧͨ��FCTRL
	Delay(10);	//10ms �ȴ��ȶ�ʱ��
	Open_FCTRL_Chansel(0,channel);//�رռ������أ�����Chansel
	Delay(50);	//50ms �ȴ��ȶ�ʱ��
	
	/*********************���ݲ���******************************/
	TIM2_Enable();             //�򿪲���
	while(!CaptureFinishFlag);//�ȴ��������
	CaptureFinishFlag = 0;
	OpenSampleLed(0);
	/*********************�����һ·����******************************/   
	if(channel)
	{
		for( i=0; i<DATALEN; i++)  //��������
		{
			if(databuf2[i] > databuf2[i+1]){databuf2[i+1] = databuf2[i]+1;}//���˴�������
			databuf2[i] = databuf2[i+1]-databuf2[i]; //�õ�ÿ����������ʱ������
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
		if((db < 35) || (err_cnt > 10))return ERROR;//����ʧ���򷵻ش���		
		return SUCCESS;//���سɹ�
	}
return ERROR;
}

/*******************************************************************************
* Function Name  : get_temp_analog
* Description    : Ӧ��ģ���¶��źŲɼ���ͨ��AD�ɼ����ŵ�ѹ������������ȵ����Ӧ���¶�����
	$T = 1/(A + B\ln{R_{temp}} + C(\ln{R_{temp}})3) - 273$

	where A=1.4051 x 10^(-3) when temperature is between -50C and 50C, B=2.369 x 10^(-4), C=1.019 x 10^(-7).

	The board measures the voltage output (at the port PADC1) of the above circuit for the temperature. R21, R23 and R26 are set with 4K ohm, R25 is set for 80 ohm and R22 is set for 0 ohm. The TEMP_PORT is connected to $R_{temp}$. The formula is the following:

	$R_{temp} = \dfrac{R_1R_2V_r + R_12V_{out}}{(R_1+R_2)V_r - R_1V_{out}}$

	Vr is the reference voltage, it is set to 1024( 3V where also used to connect to the ADC, which returns 1024 in digital format), 
	R1 is 4K ohm and R2 is 80 ohm
	Vout is the reading from the data packet

	Please refer to the following link for constructed excel sheet \\testbed.eenics.in\vmPublic_0__rw\Documents\�������ͽڵ��·\Ӧ��\Ӧ�䴫�����¶ȵ������.xlsx.
	http://www.sciweavers.org/free-online-latex-equation-editor   ����ҳ��ת��������ʽ ��������ʽ
* Input          : ͨ������1��2��3��4��
* Output         : None
* Return         : None
*******************************************************************************/
#define TEM_R1  30000
#define TEM_R2  80.6
#define VREF    65535
#define Temp_BUF_SIZE	5	//�ضȒ�Ӿ���
int32_t get_temp_analog(char i,unsigned char Channel)//����1234
{
		double adval,rtemp,temp,logrtemp;
		double temp_sum;
		int j;
		static unsigned short int temp_buf[Temp_BUF_SIZE];

		//�л�Ϊģ��ɼ����ж����ݣ�����������ֲɼ������ж����ݣ����򷵻�һ���ܴ�ĸ�����
		
		TEMP_Switch_A;             //�л�Ϊģ���¶ȴ�����
		AD7798_Init();             //��ʼ���¶Ȳɼ�ģ��
		Open_Temp_Channel(Channel);//�򿪶�Ӧͨ���¶ȴ������ĵ�
		Delay(30);				   //��ʱ��֤�����ȶ�
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
		printf("ģ���¶�%d:%f\r\n", Channel,temp_data);
#endif
     return SUCCESS;
}

/*******************************************************************************
* Function Name  : get_temp_ds18b20
* Description    : ��ȡds18b20�¶ȴ����������ݣ�
* Input          : ͨ������1��2��3��4��
* Output         : None
* Return         : None
*******************************************************************************/
int32_t get_temp_ds18b20(unsigned char Channel)//����1234
{
	TEMP_Switch_D;//�л�Ϊ�����¶�
	Open_Temp_Channel(Channel);	
	Delay(30);					 //��ʱ30ms����֤�����ȶ�
	temp_data=DS18B20_TEMP();
#ifdef	Debug_EN	
	printf("�����¶�%f\r\n",temp_data);
#endif
	
	return SUCCESS;
}

/*******************************************************************************
* Function Name  : get_stress
* Description    : �����źŲɼ���Ԫ
* Input          : ͨ������1��2��3��4��
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t get_stress(unsigned char Channel)//����1234
{
	char RetryVal;
	/***********��ִ�м�������**********************/
	for(RetryVal = 0;RetryVal < RETRY;RetryVal++)//����ʧ��������
	{		
		//Delay(5);//��ʱ�ȴ���Դ�ȶ�
		OpenSampleLed(Channel);
		PWM_Enable();//ʹ��pwm���
		DelayTimeFlag = 1;//��ʼ����
#ifdef	Debug_EN							
		printf("��%d�β���\r\n",RetryVal+1);
#endif
		if(sample_stress_channel(Channel) == SUCCESS)break;//Ӧ���źŲɼ������-->TPX	
		else if(RetryVal == 2)
		{
			Stress_Flag=0x01;
#ifdef	Debug_EN
			printf("      ͨ��%d      ���Ҳ���ʧ�ܣ�\r\n",Channel);//���Դ���������ʧ����ֵ0
#endif			
		}
	}
	PWM_Disable();

  return SUCCESS;
}

/*******************************************************************************
* Function Name  : DataPrepare
* Description    : ����׼�����������ڽ��ɼ������е�����ת�浽���ͻ�������
* Input          : ͨ������1��2��3��4
* Output         : None
* Return         : None
*******************************************************************************/
void DataPrepare(unsigned char Channel)//����1234
{
		Send_Buff.Sensor_Data[Channel-1].Sample_Status                    = 0x00;
		Send_Buff.Frame_Length                                            = sizeof(Send_Buff);  //������Ҫ����֡���ݵĳ���
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
* Description    : ָ���麯�������ڼ���������͵�����ʱ����Ч
* Input          : None
* Output         : None
* Return         : �ɹ���1������0
*******************************************************************************/
unsigned char Instruction_Check(void)
{
	Receive_CfgDATA.SampleType= I2C1_Buffer_Rx[1];
	Receive_CfgDATA.Ch         = I2C1_Buffer_Rx[2];
	Receive_CfgDATA.SensorType= I2C1_Buffer_Rx[3];
	
	if(Receive_CfgDATA.SampleType==0)return SUCCESS;//�����ģʽ0��ֱ�ӷ��سɹ�
	else if( (Receive_CfgDATA.SampleType<=2)  && (Receive_CfgDATA.Ch&0x0f) )//���������λ
		return SUCCESS;
	else
	{
		Send_Buff.Command_Status=0x01;
		return ERROR;
	}		
}

/*******************************************************************************
* Function Name  : Auto_Smaple
* Description    : �Զ�����ģʽ����ģʽ�£��ӻ��Զ�ʶ���ĸ�ͨ����������װ������
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
		(void)get_temp_analog(0,i);	//ִ��ģ���¶Ȳ�������ִ��ģ��������ɹ���λ��0  
		if(temp_data<TEMPSENSOR_LIMIT)				//ʧ��ִ�����ֲ������ɹ���λ��1
		{	
			(void)get_temp_ds18b20(i);//��ִ�������¶Ȳ���
			 SensorType_Flag = 0x01;        
		}
		if(temp_data<TEMPSENSOR_LIMIT)
		{
			TEMP_Flag=0x01;//����¶Ȳ���ʧ��//��ʧ��
#ifdef	Debug_EN
			printf("          ͨ��%d�¶Ȳ���ʧ��\r\n",i);
#endif
		}
		/***********��ִ�м�������**********************/
		if(!TEMP_Flag)
			{
				(void)(get_stress(i));
				DataPrepare(i);
			}
	}
}

/*******************************************************************************
* Function Name  : Half_Auto_Sample
* Description    : ���Զ�����ģʽ����ģʽ�£��ӻ��Զ�����ͨ��ʹ�����Զ��ж�����������
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
			(void)get_temp_analog(0,i);	//ִ��ģ���¶Ȳ�������ִ��ģ��������ɹ���λ��0  
			if(temp_data<TEMPSENSOR_LIMIT)				//ʧ��ִ�����ֲ������ɹ���λ��1
			{	
				(void)get_temp_ds18b20(i);//��ִ�������¶Ȳ���
				SensorType_Flag = 0x01;        
			}
			if(temp_data<TEMPSENSOR_LIMIT)
			{
				TEMP_Flag=0x01;//����¶Ȳ���ʧ��//��ʧ��
#ifdef	Debug_EN
				printf("          ͨ��%d�¶Ȳ���ʧ��\r\n",i);
#endif
			}
			/***********��ִ�м�������**********************/
			(void)(get_stress(i));
			DataPrepare(i);	
		}
		DATA_tmp=DATA_tmp<<1;
	}
	DATA_tmp=0x01;
}

/*******************************************************************************
* Function Name  : Manual_Sample
* Description    : �ֶ�����ģʽ����ģʽ�£��ӻ��Զ�����ͨ��ʹ�ܡ�������������������
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
				(void)get_temp_ds18b20(i);//��ִ�������¶Ȳ���//ִ�����ֲ���
				SensorType_Flag = 0x01; 
			}
			else
				(void)get_temp_analog(0,i);//ִ��ģ�����
			if(temp_data<TEMPSENSOR_LIMIT)
			{
				TEMP_Flag=0x01;//����¶Ȳ���ʧ��//��ʧ��
#ifdef	Debug_EN
				printf("          ͨ��%d�¶Ȳ���ʧ��\r\n",i);
#endif
			}
			
		/***********��ִ�м�������**********************/	
		(void)(get_stress(i));	
		DataPrepare(i);	
		}			
		DATA_tmp=DATA_tmp<<1;	//	
	}
	DATA_tmp=0x01;
}

/*******************************************************************************
* Function Name  : Process
* Description    : �������̺���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Process(void)
{
	if(Receive_CfgDATA.SampleType==0)      Auto_Smaple();            //�Զ�ģʽ��ÿ��ͨ���Զ��ж��Ƿ��д����������������ͣ�
			
	else if(Receive_CfgDATA.SampleType==1) Half_Auto_Sample();       //���Զ�ģʽ����ѡ���ͨ���б𴫸������ͣ�
			
	else				                   Manual_Sample();          //�ֶ�ģʽ����ѡ���ͨ�������������Ͳ�����
}

/*******************************************************************************
* Function Name  : ������
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
	/******************��������***********************/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//�ж����ȼ�����
	if (SysTick_Config(SystemCoreClock / 1000))while(1);	
	TIM6_Configuration();     //TIM6  ��ʱ
	NVIC_TIM6_Configuration();//TIM6�ж�����
	GPIO_Configuration();     //GPIO
	I2C1_Init();
#ifdef  Debug_EN
	UART3_Configuration(115200);//UART3  ���Դ�ӡ
#endif
	SPI1_Configuration();
	TIM2_Configuration();
	PWM_Configuration(6000,50);
	
#ifdef	Debug_EN
	printf("Hardware had ready!\r\n");
#endif
  //��ʼ��״̬����
	Send_Buff.Sensor_Data[0].Sample_Status                    = 0x03;
	Send_Buff.Sensor_Data[1].Sample_Status                    = 0x03;
	Send_Buff.Sensor_Data[2].Sample_Status                    = 0x03;
	Send_Buff.Sensor_Data[3].Sample_Status                    = 0x03;
	
  while(!I2C_Received_Flag);      //�ȴ��������Ͳ���ָ��
	(void)Instruction_Check();		//ָ���麯��
//	Receive_CfgDATA.SampleType= 2;
//	Receive_CfgDATA.Ch        = 0x0a;
//	Receive_CfgDATA.SensorType= 2;
	
	Process();	//��������
	EXIT_OUT_H;	//�ж���λ
	OpenSampleLed(0);
	while(1);				
}


