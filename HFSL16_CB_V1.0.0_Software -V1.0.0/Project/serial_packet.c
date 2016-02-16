//#include "serial_packet.h"
#include "main.h"
//#include "uart.h"

static uint8_t receive_buffer[MTU] = {1,3,5};
static uint8_t send_buffer[MTU];
static uint8_t send_prepare[120];
static int send_prepare_counter;
static uint8_t ack_buffer[3] = {P_ACK, 0, 0};
static int send_counter;
static int send_busy = 0;

static uint16_t crc_byte(uint16_t crc, uint8_t b)
{
    uint8_t i;
    crc = crc ^ b << 8;
    i = 8;
    do
        if (crc & 0x8000)
            crc = crc << 1 ^ 0x1021;
        else
            crc = crc << 1;
    while (--i);

    return crc;
}
static uint16_t crc_packet(uint8_t *data, int len)
{
    uint16_t crc = 0;

    while (len-- > 0)
        crc = crc_byte(crc, *data++);

    return crc;
}
uint8_t *read_serial_packet(int *len)
{
    unsigned char in_sync; //记录是否已经同步记录
    unsigned char escaped; //记录上次是否是 ESCAPED码
    int receive_count;     //
    uint8_t byte;
		uint16_t i;
    
    in_sync = 0;// false
    escaped = 0;// false
    receive_count = 0;

    for (;;)
        {
          while(1)
					{
						i = USART1_GetChar();
						if(i != EMPTY)
						{							
							byte = (uint8_t)i;
							break;//读取一个byte
						}
#ifdef WatchDog_EN
	IWDG_ReloadCounter();//Î¹¹·
#endif						
						//PWR_EnterSleepMode(PWR_Regulator_LowPower,PWR_SLEEPEntry_WFI);
						__WFI;
					}

            /* 
             * 如果不是同步状态，收到同步头就进入同步状态
             */
            if(!in_sync)
                {
                    if (byte == SYNC_BYTE)
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                        }
                    continue;
                }
            /* 
             * 数据长度越界，放弃同步，重新寻找同步数据头
             */
            if (receive_count >= MTU)
                {
                    in_sync = 0;
                    continue;
                }
            /*
             * 同步后内部数据处理
             */
            if (escaped) //如果上一个是替代byte
                {
                    if (byte == SYNC_BYTE)/* sync byte following escape is an error, resync */
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                            continue;
                        }
                    byte ^= 0x20;
                    escaped = 0;
                }
            else if (byte == ESCAPE_BYTE)
                {
                    escaped = 1;
                    continue;
                }
            else if (byte == SYNC_BYTE)//收到最后一个数据包
                {
                    int packet_len = receive_count;
                    uint16_t read_crc, computed_crc;

                    receive_count = 0; /* ready for next packet */
                    if (packet_len < 4)
                        /* frames that are too small are ignored */
                        /* 重新开始同步 */
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                            continue;
                        }
                    read_crc = receive_buffer[packet_len - 2] | receive_buffer[packet_len - 1] << 8;
                    computed_crc = crc_packet(receive_buffer, packet_len - 2);
                    
                    if (read_crc == computed_crc) 
                        {
                            *len = packet_len;
                            return &receive_buffer[0];
                        }
                    else
                        {
                            in_sync = 1;
                            receive_count = 0;
                            escaped = 0;
                            /* We don't lose sync here. If we did, garbage on the line
                               at startup will cause loss of the first packet. */
                            continue;
                        }
                }
            if(receive_count < MTU)receive_buffer[receive_count++] = byte;
        }
}
void add_send_buffer(uint8_t byte)
{
    if ((SYNC_BYTE == byte)||(ESCAPE_BYTE == byte))
        {
            send_buffer[send_counter++] = ESCAPE_BYTE;
            send_buffer[send_counter++] = byte ^ 0x20;
        }
    else
        {
            send_buffer[send_counter++] = byte;
        }
}
int write_serial_packet(uint8_t *data, int len)
{
    uint16_t send_crc;
    uint8_t b;
    int i;
    if (len > 110){ return -1;}
    if (send_busy){ return -2;}
    //最好使用信号量来控制，暂时先用土方法
    send_busy = 1;

    send_crc = crc_packet(data, len);
    send_counter = 0;
    send_buffer[send_counter++] = SYNC_BYTE;

    for(i=0;i<len;i++)
        {
            b = *(data + i);
            add_send_buffer(b);
        }
    b = (uint8_t)(send_crc & 0x00ff);
    add_send_buffer(b); 
		b = (uint8_t)((send_crc >> 8) & 0x00ff);
    add_send_buffer(b);
    send_buffer[send_counter++] = SYNC_BYTE;
    
    USART_PutData(USART1,(uint8_t *)send_buffer, send_counter);//正常模式
		//UART1_DMA_SendData((uint8_t *)send_buffer ,send_counter);//采用DMA方式
    send_busy = 0;
    return 1;
}
uint8_t *get_active_message(int *len)
{
    uint8_t *get_data=NULL ,*pr=NULL;
    int sf_len,i;
    
    get_data = read_serial_packet(&sf_len);
#ifdef	Debug_EN
		pr = get_data;
		for(i=0;i<sf_len;i++)
		{
			printf("0x%X ",*(pr++));
		}
#endif
    if (P_PACKET_ACK == *get_data)
        {
            ack_buffer[1] = *(get_data + 1);
            write_serial_packet(ack_buffer, 3);
        }
    *len = sf_len - 3;
    return (get_data + 3);
}
int write_active_message(uint8_t *data, int len)
{
    return 1;
}
void send_message_prepare(uint8_t am_type)
{
    send_prepare[0] = P_PACKET_NO_ACK;
    send_prepare[1]++;
    send_prepare[2] = 0;
    send_prepare[3] = 0xff;
    send_prepare[4] = 0xff;
    send_prepare[5] = 0;
    send_prepare[6] = 0;
    send_prepare[7] = 0; //这个字节用于规定数据包长度
    send_prepare[8] = 0; //group id
    send_prepare[9] = am_type;
    send_prepare_counter = 10;
}
int send_message_16_add(uint16_t b2)
{
    if (send_prepare_counter > 110)
        {
            return -1;
        }
    send_message_add( (uint8_t)((b2 >> 8) & 0xff) );
    return send_message_add( (uint8_t)(b2 & 0xff) );
}
int send_message_add(uint8_t byte)
{
    /*
     * 这里进行AM message方式组包
     */ 
    if (send_prepare_counter > 110)
        {
            send_prepare_counter = 0;
            return -1;
        }
    else
        {
            send_prepare[send_prepare_counter++] = byte;
            return 1;
        }
}
int send_message(uint8_t a)
{
    if (send_prepare_counter > 9)
        {
            send_prepare[7] = (uint8_t)(send_prepare_counter - 10);
            write_serial_packet(send_prepare, send_prepare_counter);
            return 1;
        }
    else
        {
            return -1;
        }
}
