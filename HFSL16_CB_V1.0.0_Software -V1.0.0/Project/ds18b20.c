
#include "ds18b20.h"

unsigned char DS18B20_Rom_Addr[8];

void DS18B20_delayus(unsigned int us)
{
	
			//480  524us
			//80	 90us
			//60	 68.4us
			//58	 66us
			//50   57.6us
			//20   24.8us
			//15   19.4us
			//10   13.96us
			//7    10.72us
			//5    8.54us
			//4		 7.46us
			//3    6.38us
			//2    5.3us
			//1    4.2us
			//0    3.12us
			//nop  1.35us
	
 unsigned int  i=0;
	for(i=0;i<us;i++)
	{__nop();}
}

unsigned char DS18B20_Rst(void)			//复位DS18B20
{
	unsigned char dat;
	SET_DS18B20_DQ_OUT;
	GPIOC->BSRRH = GPIO_Pin_7;//拉低
	DS18B20_delayus(480);//524us
	DQ_OUT_H;
	SET_DS18B20_DQ_IN;
	DS18B20_delayus(53);//60us
	
	dat=(GPIOC->IDR & GPIO_Pin_7);
	
	DS18B20_delayus(480);//524us
	SET_DS18B20_DQ_OUT;
	DQ_OUT_H;
	return dat;	
}

unsigned char DS18B20_Read_Byte(void)
{
	unsigned char i,dat=0;
	
	for(i=0;i<8;i++)
	{
		dat>>=1;
		SET_DS18B20_DQ_OUT;//设置为输出
		
		DQ_OUT_L;					//拉低总线
		__nop();__nop();
		SET_DS18B20_DQ_IN;//设置成输入，由外部上拉电阻将总线拉高，释放总线
		__nop();
		if(GPIOC->IDR & GPIO_Pin_7)//获取端口值
			dat|=0x80;
		DS18B20_delayus(58);//66us
	}
	SET_DS18B20_DQ_OUT;
	return dat;
}

void ReadROM()
{
	unsigned char i;
	DS18B20_Write_Byte(0x33);//发送读rom命令
	
	for(i=0;i<8;i++)				//读rom
	{
		DS18B20_Rom_Addr[i]=DS18B20_Read_Byte();
	}
#ifdef	Debug_EN
	printf("Rom_Addr: ");
	for(i=0;i<8;i++)
	{
		printf("0x%x ",DS18B20_Rom_Addr[i]);
	}
	printf("\r\n");
#endif
	
}

void DS18B20_Write_Byte(unsigned char dat)
{
	unsigned char i;
	SET_DS18B20_DQ_OUT;
	for(i=0;i<8;i++)
	{
		if(dat&0x01)
		{	
			DQ_OUT_L;//写时间空隙总是从总线的低电平开始
			DS18B20_delayus(2);//15us内拉高
			DQ_OUT_H;
			DS18B20_delayus(58);//69us//整个写1时隙不低于60us
		}
		else
		{
			DQ_OUT_L;
			DS18B20_delayus(60);//保持在60-120us之间
			DQ_OUT_H;
			DS18B20_delayus(2);//
		}
		dat>>=1;
	}
}


signed short int DS18B20_TEMP(void)
{
	double tt;
	signed   int temp;
	unsigned char a,b;
	__disable_irq();
	
//	if(!DS18B20_Rst()) ;
//	ReadROM();
	
	if(DS18B20_Rst())
	{
		__enable_irq();
		return -8500;
	}
	DS18B20_Write_Byte(0xcc);
	DS18B20_Write_Byte(0x44);
	__enable_irq();
	
	Delay(1000);
	
	__disable_irq();
	if(DS18B20_Rst())
	{
		__enable_irq();
		return -8500;
	}
	DS18B20_Write_Byte(0xcc);
	DS18B20_Write_Byte(0xbe);
	a=DS18B20_Read_Byte();
	b=DS18B20_Read_Byte();
	__enable_irq();
	temp=(b<<8) | a ;

	if(temp>0x0fff)
	{
		temp=~temp+1;
		tt=temp*0.0625;
		tt=tt*1000+50;//温度数据共放大100倍，+5是四舍五入
#ifdef	Debug_EN
		printf("-%.3f℃\r\n",tt/1000);
#endif
		return (0-tt);
	}
	else 
	{
		tt=temp*0.0625;
		tt=tt*1000+50;//温度数据共放大100倍，+5是四舍五入
#ifdef	Debug_EN
		printf("temp:%.3f℃\r\n",tt/1000);
#endif
		
		return (int32_t)tt;
	}
}


