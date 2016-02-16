
#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stm32l1xx.h"
#include "time.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "InputCapture.h"
#include "ds18b20.h"


#pragma   pack(1)										    //�ṹ�嵥�ֽڶ��뷽ʽ      #pragma pack(n)

//#define   WatchDog_EN
#define		Debug_EN

#define SystemResetSoft								*((uint32_t *)0xe000ed0c)=0x05fa0004; //ʵ��ϵͳ�����λ
#define _NOP()    				__ASM("NOP")

#define EXIT_OUT_H					GPIO_SetBits(GPIOA,GPIO_Pin_0)
#define EXIT_OUT_L					GPIO_ResetBits(GPIOA,GPIO_Pin_0)

#define LED1_ON						GPIO_ResetBits(GPIOB,GPIO_Pin_4)						
#define LED1_OFF					GPIO_SetBits(GPIOB,GPIO_Pin_4)							

#define LED2_ON						GPIO_ResetBits(GPIOC,GPIO_Pin_10)						
#define LED2_OFF					GPIO_SetBits(GPIOC,GPIO_Pin_10)							

#define LED3_ON						GPIO_ResetBits(GPIOC,GPIO_Pin_8)						
#define LED3_OFF					GPIO_SetBits(GPIOC,GPIO_Pin_8)							

#define LED4_ON						GPIO_ResetBits(GPIOB,GPIO_Pin_2)						
#define LED4_OFF					GPIO_SetBits(GPIOB,GPIO_Pin_2)		


#define FCTR1_H						GPIOB->BSRRL = GPIO_Pin_5
#define FCTR1_L						GPIOB->BSRRH = GPIO_Pin_5

#define FCTR2_H						GPIOC->BSRRL = GPIO_Pin_11
#define FCTR2_L						GPIOC->BSRRH = GPIO_Pin_11

#define FCTR3_H						GPIOA->BSRRL = GPIO_Pin_8
#define FCTR3_L						GPIOA->BSRRH = GPIO_Pin_8

#define FCTR4_H						GPIOC->BSRRL = GPIO_Pin_5
#define FCTR4_L						GPIOC->BSRRH = GPIO_Pin_5


#define CHANSEL1_H					GPIOB->BSRRL = GPIO_Pin_6
#define CHANSEL1_L					GPIOB->BSRRH = GPIO_Pin_6

#define CHANSEL2_H					GPIOC->BSRRL = GPIO_Pin_12
#define CHANSEL2_L					GPIOC->BSRRH = GPIO_Pin_12

#define CHANSEL3_H					GPIOC->BSRRL = GPIO_Pin_9
#define CHANSEL3_L					GPIOC->BSRRH = GPIO_Pin_9

#define CHANSEL4_H					GPIOB->BSRRL = GPIO_Pin_1
#define CHANSEL4_L					GPIOB->BSRRH = GPIO_Pin_1


#define TEMP_P0_H					GPIO_SetBits(GPIOC,GPIO_Pin_0)
#define TEMP_P0_L					GPIO_ResetBits(GPIOC,GPIO_Pin_0)

#define TEMP_P1_H					GPIO_SetBits(GPIOC,GPIO_Pin_1)
#define TEMP_P1_L					GPIO_ResetBits(GPIOC,GPIO_Pin_1)

#define TEMP_P2_H					GPIO_SetBits(GPIOC,GPIO_Pin_2)
#define TEMP_P2_L					GPIO_ResetBits(GPIOC,GPIO_Pin_2)

#define TEMP_Switch_A				GPIO_ResetBits(GPIOC,GPIO_Pin_6)
#define TEMP_Switch_D				GPIO_SetBits(GPIOC,GPIO_Pin_6)

//DS18B20������   PC7
#define DQ_OUT_H					GPIOC->BSRRL = GPIO_Pin_7
#define DQ_OUT_L					GPIOC->BSRRH = GPIO_Pin_7

//����DS18B20�����߷���
#define SET_DS18B20_DQ_IN     GPIOC->MODER   &= 0xffff3fff;\
							  GPIOC->PUPDR   &= 0xffff3fff;GPIOC->PUPDR   |= 0x00004000
															//�������빦��
															//��������
#define SET_DS18B20_DQ_OUT    GPIOC->MODER   &= 0xffff3fff;GPIOC->MODER   |= 0x00004000;\
							  GPIOC->OTYPER  &= 0xffffff7f;GPIOC->OTYPER  |= 0x00000080;\
							  GPIOC->OSPEEDR &= 0xffff3fff;GPIOC->OSPEEDR |= 0x0000c000;\
							  GPIOC->PUPDR   &= 0xffff3fff;GPIOC->PUPDR   |= 0x00004000
							//����Ϊ�������
							//�����������Ϊ��©
							//��������ٶ�Ϊ40MHz
							//��������


#define htons(n) ((((n) & 0x00ff) << 8) | (((n) & 0xff00) >> 8))
#define ntohs(n) htons(n)
#define htonl(n) ((((n) & 0x000000ff) << 24) |  \
                  (((n) & 0x0000ff00) << 8)  |  \
                  (((n) & 0x00ff0000) >> 8)  |  \
                  (((n) & 0xff000000) >> 24))
#define ntohl(n) htonl(n)


#define I2C_END_RECEIVED         ((uint32_t)0x00000050)

void GPIO_Configuration(void);

void GPIO_PinReverse(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void Delay(__IO uint32_t nTime);

typedef struct{
	
	unsigned char Sensor_Type;
	unsigned char Sample_Status;
	unsigned int  Freq_Value;
	unsigned char DB_Value;
	int16_t       Err_Value;
	int           Temp_Value;
	
}Send_ChDATA_tagdef;

typedef struct{
	
	unsigned char SampleType;	//�������ͻ��棬���ڽ���I2C���͹����ĵĲ�����������
	unsigned char Ch;			//����ͨ�����棬���ڽ���I2C���͹�����ͨ������
	unsigned char SensorType;	//���崫�������ͻ��棬���ڽ���I2C���͹����Ĵ�������������
}Receive_CfgDATA_tagdef;


typedef struct{
	
	unsigned char        Frame_Length;
	unsigned char        Command_Status;
	Send_ChDATA_tagdef   Sensor_Data[4];
	
}Send_Buff_tagdef;

extern __IO uint32_t TimingDelay;
extern __IO unsigned char Slave_TxBufLen;
extern __IO unsigned char *Pr_Tx;
extern __IO unsigned char I2C1_Buffer_Rx[64];
extern Send_Buff_tagdef      Send_Buff;
extern uint16_t 	DelayTime;//��ʱ����ֵ(ms)
extern uint8_t 	DelayTimeFlag;//��ʱ��־
extern unsigned char IS_I2C_Received_Completed;

extern unsigned int  SysTick_Count;


extern unsigned char *data_p;






#endif


