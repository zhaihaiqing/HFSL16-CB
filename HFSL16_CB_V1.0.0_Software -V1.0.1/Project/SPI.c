#include "main.h"
#include "spi.h"
/*******************************************************************************
* Function Name  : SPI1_Configuration
* SPI1初始化
*******************************************************************************/
void SPI1_Configuration(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
  

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	/*PA5=SCK1,PA6=MISO1,PA7=MOSI1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 |GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*  PA4=CS1*/
	GPIO_SetBits(GPIOA, GPIO_Pin_4);//预置为高
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* SPI1 configuration */
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//主
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
		
	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_Cmd(SPI1, ENABLE); 
}
/*******************************************************************************
* Function Name  : AD7798_delay
* val:延时数据
*******************************************************************************/
void AD7798_delay(unsigned int val)
{
  unsigned int i;
  i = 1000*val;
  while(i--);
}
/*******************************************************************************
* Function Name  : SPI2_FlashReadWriteByte
* 从AD7798 读写一个数据
*******************************************************************************/

unsigned char SPI1_ReadWriteByte(unsigned char byte)
{
  unsigned int timeout = SPI_FLAG_TimeOut;
  /*!< Loop while DR register in not emplty */
  while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) && (timeout--));

  /*!< Send byte through the SPI2 peripheral */
  SPI_I2S_SendData(SPI1, byte);
  
   /*!< Wait to receive a byte */
  timeout = SPI_FLAG_TimeOut;
  while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) && (timeout--));

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);

}
/*******************************************************************************
* Function Name  : AD7798_Reset
* AD7798 reset
*******************************************************************************/
static void AD7798_Reset(void)
{
  char i;
  //AD_SPI_CS_LOW; 
  for(i = 0; i<4; i++)
   SPI1_ReadWriteByte(0xff);
  //AD_SPI_CS_HIGH;
  Delay(10);
}
/*******************************************************************************
* Function Name  : AD7798_Comm
* rw:读操作=1，写操作=2
* reg：寄存器地址
* cread：是否连续读 1=连续读
* AD7798 reset
*******************************************************************************/
static void AD7798_Comm(unsigned char reg, unsigned char rw, unsigned char cread)
{
   SPI1_ReadWriteByte((rw ? 0x40 : 0x00) | (reg << 3) | (cread ? 0x04 : 0x00));
}
/*******************************************************************************
* Function Name  : AD7798_StatusRegisterRead
* 读取AD7798状态寄存器
*******************************************************************************/
unsigned char AD7798_StatusRegisterRead(void)
{ 
  unsigned char Status; 
 
  //AD7798_Reset(); //复位
  //AD_SPI_CS_LOW;
  AD7798_Comm(AD7798_STATUS_REG,1,0);  //读寄存器操作
  Status=SPI1_ReadWriteByte(Dummy_Byte);  //读取
  //AD_SPI_CS_HIGH; 
  return Status;     
}
/*******************************************************************************
* Function Name  : AD7798_ReadOffset
* 读取AD7798校准寄存器
*******************************************************************************/
unsigned int AD7798_ReadOffset(void)
{
  unsigned int val = 0; 
  AD7798_Comm(AD7798_OFFSET_REG, 1, 0);
   val = (unsigned int)SPI1_ReadWriteByte(Dummy_Byte) << 8;
   val |= (unsigned int)SPI1_ReadWriteByte(Dummy_Byte);
   return val;
}

/*******************************************************************************
* Function Name  : AD7798_ReadScale
* 读取AD7798满度校准寄存器
*******************************************************************************/
unsigned int AD7798_ReadScale(void)
{
  unsigned int val = 0;  
  AD7798_Comm(AD7798_SCALE_REG, 1, 0);
   val = (unsigned int)SPI1_ReadWriteByte(Dummy_Byte) << 8;
   val |= (unsigned int)SPI1_ReadWriteByte(Dummy_Byte);
   return val;
}
/*******************************************************************************
* Function Name  : AD7798_SetMode
* 设置AD7798工作模式
* mode:模式选择
* psw:电源开关
* rate:转换速率
*******************************************************************************/
void AD7798_SetMode(unsigned char mode, unsigned char psw, unsigned char rate)
{
    AD7798_Comm(AD7798_MODE_REG, 0, 0);
    SPI1_ReadWriteByte(mode << 5 | (psw ? 0x10 : 0x00));
    SPI1_ReadWriteByte(rate);
}
/*******************************************************************************
* Function Name  : AD7798_WriteConfig
* 写AD7798配置寄存器
* burnout:电流使能位
* UB:单/双极选择位
* gain:增益
* ref_det:参考保护使能
* buf:配置ADC是否使用内部缓冲
* channel:通道选择
*******************************************************************************/
void AD7798_WriteConfig(unsigned char burnout, unsigned char UB, unsigned char gain,unsigned char ref_det, unsigned char buf, unsigned char channel)
{
   AD7798_Comm(AD7798_CONFIG_REG, 0, 0);
   SPI1_ReadWriteByte((burnout ? 0x20 : 0x00) | (UB ? 0x10 : 0x00) | gain);
   SPI1_ReadWriteByte((ref_det ? 0x20 : 0x00) | (buf ? 0x10 : 0x00) | channel);
}
/*******************************************************************************
* Function Name  : AD7798_DataReady
* 检查AD7798是否准备好
*******************************************************************************/
unsigned char AD7798_DataReady(void)
{
   return (!AD7798_RDY);
}
/*******************************************************************************
* Function Name  : AD7798_RequestData
* 设置是否连续读数据
* continuous:1=连续读,0=读一次
*******************************************************************************/
void AD7798_RequestData(unsigned char continuous)
{
   AD7798_Comm(AD7798_DATA_REG, 1, continuous);
}
/*******************************************************************************
* Function Name  : AD7798_ReadData
* 读数据
*******************************************************************************/
unsigned int AD7798_ReadData(char channel)
{
   unsigned int val = 0;
  AD_SPI_CS_LOW; 
  AD7798_SetMode(AD7798_SINGLE_CONVERSION_MODE,0,AD7798_16_7_1_HZ);
   AD7798_WriteConfig(0,1,AD7798_1_GAIN,0,0,channel);
   AD7798_RequestData(0);
    while(!AD7798_DataReady())Delay(10);;
  val = SPI1_ReadWriteByte(0);
   val <<= 8;
   val |= SPI1_ReadWriteByte(0);
   AD_SPI_CS_HIGH;
   return (val - 0x8000);
}
/*******************************************************************************
* Function Name  : AD7798_ContinuousReadData
* 读数据
* channel:通道
* *databuf：数据指针
* number：读取数量
* 返回：0：超时，1：成功
*******************************************************************************/
unsigned char AD7798_ContinuousReadData(char channel,unsigned short int *databuf,unsigned int number)
{
  unsigned int val = 0,timeout = 1000,i;
  AD_SPI_CS_LOW; 
  AD7798_SetMode(AD7798_CONTINUOUS_CONVERSION_MODE,0,AD7798_123_HZ);
  AD7798_WriteConfig(0,1,AD7798_1_GAIN,0,0,channel);
   //AD7798_RequestData(1); //连续读取
  //Delay(50);
	for(i = 0,timeout = 1000;i < number;i++)
  {  
		while(!AD7798_DataReady())
    {
      Delay(2);
      if(!(--timeout))return 0; //超时则返回0  1s
    }
		AD7798_RequestData(0); 
		val = SPI1_ReadWriteByte(0);
    val <<= 8;
    val |= SPI1_ReadWriteByte(0);
    databuf[i] = val;
    //log_info("channel(%d)databuf[%d]=0x%d",channel,i,databuf[i]);
  }
  AD_SPI_CS_HIGH;
    return 1;
}

/*******************************************************************************
* Function Name  : AD7798_Calibrate
* 校正
*******************************************************************************/
unsigned int AD7798_Calibrate(void)
{
   unsigned int offset;
   offset  = AD7798_ReadOffset();
  //log_info("offset=0x%d",offset); 
  /* cal */
   AD7798_SetMode(AD7798_INTERNAL_OFFSET_CAL_MODE, 0, AD7798_16_7_1_HZ);
   while(!AD7798_DataReady())
     Delay(10);
   offset = AD7798_ReadOffset();
   //log_info("offset=0x%d",offset);
   return offset;
}
/*******************************************************************************
* Function Name  : AD7798_Init
* 初始化AD7798
*******************************************************************************/
unsigned char AD7798_Init(void)
{
  unsigned char Status; 
  AD_SPI_CS_LOW;
  AD7798_Reset();
  Status = AD7798_StatusRegisterRead();
  AD7798_Calibrate();
  //AD7798_SetMode(AD7798_CONTINUOUS_CONVERSION_MODE,0,AD7798_16_7_1_HZ);
  AD_SPI_CS_HIGH;
  return Status;
}
