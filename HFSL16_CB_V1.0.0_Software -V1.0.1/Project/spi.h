
#ifndef __SPI_H
#define __SPI_H

#define SPI_FLAG_TimeOut  0x5000   //超时常量
#define SPI_LONG_TimeOut  (10 * SPI_FLAG_TimeOut)

#define Dummy_Byte 0x00  //不考虑的数据

#define AD_SPI_CS_HIGH    GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define AD_SPI_CS_LOW     GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define AD7798_RDY        GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)

#define AD7798_STATUS_RDY       0x80
#define AD7798_STATUS_ERROR     0x40
#define AD7798_STATUS_NOREF     0x20
#define AD7798_STATUS_CHAN_MASK 0x07

#define AD7798_STATUS_REG 0x0
#define AD7798_MODE_REG   0x1
#define AD7798_CONFIG_REG 0x2
#define AD7798_DATA_REG   0x3
#define AD7798_ID_REG     0x4
#define AD7798_IO_REG     0x5
#define AD7798_OFFSET_REG 0x6
#define AD7798_SCALE_REG  0x7

enum {  AD7798_CONTINUOUS_CONVERSION_MODE,  
        AD7798_SINGLE_CONVERSION_MODE,
        AD7798_IDLE_MODE, 
        AD7798_POWERDOWN_MODE, 
        AD7798_INTERNAL_OFFSET_CAL_MODE,
        AD7798_INTERNAL_SCALE_CAL_MODE,  
        AD7798_SYSTEM_OFFSET_CAL_MODE,
        AD7798_SYSTEM_SCALE_CAL_MODE
      };

enum {  AD7798_470_HZ = 1, 
        AD7798_242_HZ, 
        AD7798_123_HZ, 
        AD7798_62_HZ,
        AD7798_50_HZ, 
        AD7798_39_HZ, 
        AD7798_33_2_HZ, 
        AD7798_19_6_HZ,
        AD7798_16_7_1_HZ, 
        AD7798_16_7_2_HZ, 
        AD7798_12_5_HZ, 
        AD7798_10_HZ,
        AD7798_8_33_HZ, 
        AD7798_6_25_HZ, 
        AD7798_4_17_HZ 
     };
enum {  AD7798_1_GAIN, 
        AD7798_2_GAIN, 
        AD7798_4_GAIN, 
        AD7798_8_GAIN,
        AD7798_16_GAIN, 
        AD7798_32_GAIN, 
        AD7798_64_GAIN, 
        AD7798_128_GAIN 
      };
enum {  AD7798_AIN1_CHAN, 
        AD7798_AIN2_CHAN, 
        AD7798_AIN3_CHAN,
        AD7798_AIN11_CHAN
      };

void SPI1_Configuration(void);
unsigned char AD7798_Init(void);
unsigned char AD7798_ContinuousReadData(char channel,unsigned short int *databuf,unsigned int number);


#endif


