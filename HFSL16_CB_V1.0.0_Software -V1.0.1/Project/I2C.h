#include <stm32l1xx_i2c.h>


//器件采用7位地址模式
#define CB1_ADDR						0x30			//CB地址；格式：0 0 1 1  A2 A1 A0 un
#define CB2_ADDR						0x32
#define CB3_ADDR						0x34
#define CB4_ADDR						0x38
#define I2C1_MBOWN_ADDRESS7   0x20     //MB自身地址

#define ClockSpeed            200000   //速率200K
#define I2C_FLAG_TimeOut  		0xffff     //超时常量 0x5000
#define I2C_LONG_TimeOut  		(10 * I2C_FLAG_TimeOut)

extern __IO unsigned char I2C_Received_Flag;

void I2C1_Init(void);



