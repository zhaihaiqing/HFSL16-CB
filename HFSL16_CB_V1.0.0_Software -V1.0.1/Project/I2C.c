
#include "main.h"

unsigned char CB_ADDR;
__IO unsigned char I2C_Received_Flag=0x00;
/*******************************************************************************
* Function Name  : I2C1_Configuration
* 
*******************************************************************************/
void I2C1_SlaveAddrConfig()
{
	unsigned char dat;
	GPIO_InitTypeDef        GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	//��ȡԤ���ַ  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	dat = GPIO_ReadInputData(GPIOB)>>12;
	switch(dat)
	{
		case 1:
			CB_ADDR=CB1_ADDR;
			break;
		case 2:
			CB_ADDR=CB2_ADDR;
			break;
		case 3:
			CB_ADDR=CB3_ADDR;
			break;
		case 4:
			CB_ADDR=CB4_ADDR;
			break;
		default:
			break;
	}
}

/*******************************************************************************
* Function Name  : I2C1_NVICConfig NVIC����
* 
*******************************************************************************/
void I2C1_NVICConfig()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Reconfigure and enable I2C1 error interrupt to have the higher priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Configure and enable I2C1 error interrupt -------------------------------*/
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : I2C1_Configuration
* 
*******************************************************************************/
void I2C1_Configuration()
{
	//����GPIO�ṹ�壬����I2C�ṹ��
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStructure;
	
	/* Enable I2C1 ��GPIO clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	//���������ӵ���Ӧ�˿���
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���踴��ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//��©���
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
		
	/* I2C1 configuration ------------------------------------------------------*/
	I2C_DeInit(I2C1);//I2C��ʼ��
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;//����ΪI2Cģʽ
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;//������ֻ����I2C�����ڿ���ģʽ�£�ʱ�ӹ���Ƶ�ʸ���100KHz���²�������
	I2C_InitStructure.I2C_OwnAddress1 = CB_ADDR;//��������ĵ�ַ
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;//ʹ��Ӧ��
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;//����Ϊ7λ��ַģʽ
	I2C_InitStructure.I2C_ClockSpeed = ClockSpeed;//����ʱ���ٶȣ�
	I2C_AcknowledgeConfig(I2C1, ENABLE);  //����1�ֽ�1Ӧ��ģʽ
	I2C_Init(I2C1, &I2C_InitStructure);
	
	I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_ERR | I2C_IT_BUF , ENABLE);
	I2C_Cmd(I2C1, ENABLE);
}

/*******************************************************************************
* Function Name  : I2C1_Init I2C��ʼ������
* 
*******************************************************************************/
void I2C1_Init(void)
{
	I2C1_SlaveAddrConfig();
	I2C1_NVICConfig();
	I2C1_Configuration();
}




