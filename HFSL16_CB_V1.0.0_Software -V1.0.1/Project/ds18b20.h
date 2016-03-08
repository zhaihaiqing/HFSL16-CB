#ifndef __DS18B20_H
#define __DS18B20_H


#include <main.h>
#include <stm32l1xx.h>
#include <stm32l1xx_gpio.h>


void DS18B20_delayus(unsigned int us);
unsigned char DS18B20_Rst(void);
unsigned char DS18B20_Read_Bit(void);
unsigned char DS18B20_Read_Byte(void);															
void DS18B20_Write_Byte(unsigned char dat);															
void ReadROM();														
float DS18B20_TEMP(void);

															
															
															
															
#endif
	














