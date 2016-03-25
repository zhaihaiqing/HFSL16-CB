#ifndef INPUTCAPTURE_H
#define INPUTCAPTURE_H

#define	DATALEN	500

extern  volatile unsigned      int CaptureFlag1,CaptureFlag2,CaptureFlag3;
extern  volatile unsigned      int CaptureNumber;
extern  volatile unsigned      int CaptureFinishFlag;
extern  volatile unsigned      int databuf1[],databuf2[],databuf3[];
extern  volatile unsigned    	 int CaptureChannelFlag;

void TIM2_Configuration(void);
void TIM2_Enable(void);
void TIM2_Disable(void);
void NVIC_TIM2_Configuration(void);
void calc_itval_r2(volatile uint32_t ival_buf[], uint16_t buflen, float* itval, uint8_t* db);
uint16_t interval_filter(volatile uint32_t in_buf[], uint16_t buflen, float* itval, uint8_t* db);



#endif

