/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "InputCapture.h"

volatile unsigned      int CaptureFlag1 = 0,CaptureFlag2 = 0,CaptureFlag3 = 0;
volatile unsigned      int CaptureNumber = 0;
volatile unsigned      int CaptureFinishFlag = 0;      //捕获完成标志位
volatile unsigned	   int databuf2[DATALEN+1];
volatile unsigned      int CaptureChannelFlag = 0;//定义当前采样通道

/**********************************************************************************
* Function Name  : TIM2_Configuration
* TIM2初始化
**********************************************************************************/
void TIM2_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	 /* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	

  /* Enable the TIM1 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; //捕获中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占式  最高优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	/* TIM2 channel 2,3,4 pin (PA1,PA2,PA3) configuration */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2 ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);
}
/**********************************************************************************
* Function Name  : TIM2_Enable
* TIM2使能,开始捕获
**********************************************************************************/
void TIM2_Enable()
{
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	
	TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	
 
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;//捕获通道使能
	TIM_ICInit(TIM2, &TIM_ICInitStructure);
	TIM_ITConfig(TIM2,  TIM_IT_CC3 | TIM_IT_Update, ENABLE); 
	CaptureChannelFlag = 2;

	//TIM2->PSC = 3;//分频值  

  /* TIM enable counter */
  TIM_Cmd(TIM2, ENABLE);  //使能timer 2
  /* Enable the CC2 Interrupt Request */
 	//TIM_ITConfig(TIM2,  TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4 | TIM_IT_Update, ENABLE);  //使能 捕获/比较   更新  中断源
}
/**********************************************************************************
* Function Name  : TIM2_Disable
* TIM2关闭,停止捕获
**********************************************************************************/
void TIM2_Disable(void)
{
  /* Enable the CC2 Interrupt Request */
  TIM_ITConfig(TIM2,  TIM_IT_CC3 | TIM_IT_Update, DISABLE);  //使能 捕获/比较1   更新  中断源
  /* TIM enable counter */
  TIM_Cmd(TIM2, DISABLE);  //
}
#define MODE_MAXVAL 32 
#define MODE_MAXORDER 5 // 32 = 2^5

#define FILTER_UBOUND(x) (1.2*x)
#define FILTER_LBOUND(x) (0.8*x)

static uint8_t __log2(uint32_t x) {
    uint8_t ans = 0; 
    while (x >>= 1) ans++;
    return ans;
}

/*
 * Find the "center" of clock interval of in_buf
 *
 */
 static uint32_t find_mode(volatile uint32_t in_buf[], int buflen) {
    uint8_t count[MODE_MAXVAL];
    uint32_t i, _max=0, _min=UINT32_MAX, lower=0;
    uint32_t sample_odr;

    for (i = 0; i < MODE_MAXVAL; i++) {
        count[i] = 0;
    }

    // find dynamic range
    for (i = 0; i < buflen; i++) {
         if (in_buf[i] > _max) { 
            _max = in_buf[i];
         }
         if (in_buf[i] < _min) {
             _min = in_buf[i];
         }
    }

    // find desampling order 
    sample_odr = __log2((_max - _min) >> MODE_MAXORDER) + 1;

    // count
    for (i = 0; i < buflen; i++) {
        int v = (in_buf[i]-_min) >> sample_odr; 
        (count[v])++;
    }

    // find max
    _max = 0;
    for (i = 0; i < 32; i++) {
         if (count[i] > _max) {
            _max = count[i]; 
            lower = i;
         }
    }

    // esitimated center interval
    return _min + (lower << sample_odr) + (1 << (sample_odr - 1));
 }

/*
 * Calculate R2 of linear regression of intervals
 *
 * Arguments:
 *  ival_buf: input buffer with clock intervals
 *  buflen: input buffer length 
 *
 * Returns(via pointer):
 *  itval: mean interval
 *  db: -10 * log10(1-r2), e.g. if r2 is 0.999, db equals to 30
 *
 */

void calc_itval_r2(volatile uint32_t ival_buf[], uint16_t buflen, uint16_t* itval, uint8_t* db) { 
    uint16_t i;
    static uint32_t buf[DATALEN], s = 0;
    float a=0, ybar=0, yhat=0, sst=0, sse=0;
		s = 0;
	for (i = 0; i < buflen; i++) {
        s += ival_buf[i];
         buf[i] = s;
        ybar += s/buflen;
    }
    a = (s - buf[0]) / buflen;

    yhat = buf[0];
    for (i = 0; i < buflen; i++) { 
        sst += (buf[i] - ybar) * (buf[i] - ybar);
        sse += (buf[i] - yhat) * (buf[i] - yhat);
        yhat += a;  // since step = 1
    }
    *itval = (uint16_t)a; 
    *db = (uint8_t)floor(10 * log10(sst / sse));
    //log_info("r^2=%f", log10(sse/sst));
}

/*
 * Filter noises in interval buffer
 *
 * Arguments:
 *  in_buf: input buffer with clock intervals 
 *  buflen: input buffer length
 *  count: a pointer to return real period count in `in_buf`
 *  db: a pointer to return the r2 of linear regression
 *
 * Returns:
 *  err_cnt: count of error periods 
 *
 */

// uint16_t interval_filter(volatile uint32_t in_buf[], uint16_t buflen, uint16_t* itval, uint8_t* db) { 
// uint16_t i, err_cnt=0, idx=0; 
// static uint32_t acc=0, cntr, lbound, ubound, flt_buf[DATALEN]; 
// acc=0;
// cntr = find_mode(in_buf, buflen); 

// lbound = (uint32_t)FILTER_LBOUND(cntr); 
// ubound = (uint32_t)FILTER_UBOUND(cntr); 

// //log_info("estimation: %d lower: %d upper:%d\n", cntr, lbound, ubound); 

// //??1:???????? 
// i = 0;	
// while(((in_buf[i] < lbound) || (in_buf[i] > ubound)) && i < DATALEN) i++; 
// for (i = i; i < buflen; i++) { 
// int V = acc + in_buf[i]; 
// while(V > ubound){	//??2:V>ubound?,???? 
// flt_buf[idx++]= cntr; 
// V-=cntr; 
// err_cnt++; 
// } 
// if ((V >= lbound) && (V <= ubound)) { 
// flt_buf[idx] = V; 
// idx++; 
// acc = 0; 
// } else if (V < lbound) { 
// acc = V; 
// } 
// } 
// calc_itval_r2(flt_buf, idx, itval, db); 

// return err_cnt; 
// } 

uint16_t interval_filter(volatile uint32_t in_buf[], uint16_t buflen, uint16_t* itval, uint8_t* db) {
    //uint16_t i, err_cnt=0, idx=0;
		 uint16_t i, err_cnt1=0, err_cnt2=0, idx=0;
    static uint32_t acc=0, cntr, lbound, ubound, flt_buf[DATALEN]; 
		acc = 0;
    cntr = find_mode(in_buf, 50);

    lbound = (uint32_t)FILTER_LBOUND(cntr);
    ubound = (uint32_t)FILTER_UBOUND(cntr);

    //log_info("estimation: %d lower: %d upper:%d\n", cntr, lbound, ubound); 

//	i = 0;	
// while(((in_buf[i] < lbound) || (in_buf[i] > ubound)) && i < DATALEN) i++; 
	
    for (i = 0; i < buflen; i++) {
        int V = acc + in_buf[i];
        if ((V >= lbound) && (V <= ubound)) {
            flt_buf[idx] = V;
             idx++;
            acc = 0;
        } else if (V < lbound) {
            acc = V;
						err_cnt1++;//毛刺计数
        } else { // V > ubound
            err_cnt2++;//分析错误
            if ((in_buf[i] >= lbound) && (in_buf[i] <= ubound)) { 
                flt_buf[idx] = in_buf[i];
                idx++;
                acc = 0;
            }
            else if (in_buf[i] < lbound) {
                acc = in_buf[i]; 
            }
            else {
                acc = 0;
            }
        }
    }
    calc_itval_r2(flt_buf, idx, itval, db);
		//printf("errcnt: %d\r\n",err_cnt);
		//printf("err_cnt1=%d\terr_cnt2=%d\r\n",err_cnt1,err_cnt2);
    //return err_cnt;
		return err_cnt2;
}




