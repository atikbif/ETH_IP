/*
 * udp_server.c
 *
 *  Created on: 13 џэт. 2019 у.
 *      Author: User
 */

#include "udp_server.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "crc.h"
#include "fsmc_flash.h"
//#include "flash_if.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include "datetime.h"

#define UDP_SERVER_PORT    7
#define INCORRECT_PAGE_NUM	1
#define READ_ERROR		2
#define PAGE_BUSY		3
#define BAD_BLOCK		4

extern RTC_HandleTypeDef hrtc;
extern unsigned long cTime;

static char answer[1324];
static struct{
	unsigned char buf[2048];
	unsigned long page_num;
	unsigned char ready;
}cache;

extern uint16_t free_mem[8];

extern unsigned short bl_num;
extern unsigned short page_num;
extern unsigned char bad_cnt;
static unsigned short cache_offset = 0;
static unsigned long pcPageNum = 0;
static unsigned short int_bl_num = 0;
static unsigned short int_page_num = 0;
static unsigned short reqID = 0;
extern unsigned short busy_cnt;

volatile uint8_t *UniqueID = (uint8_t *)0x1FFF7A10;

static RTC_TimeTypeDef sTime;
static RTC_DateTypeDef sDate;
static unsigned long curTime;


extern uint8_t read_ip_from_conf(uint8_t num);
extern uint8_t read_mask_from_conf(uint8_t num);
extern uint8_t read_gate_from_conf(uint8_t num);

unsigned short udp_tmr = 0;

static void udp_server_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void inline send_udp_data(struct udp_pcb *upcb,const ip_addr_t *addr,u16_t port,u16_t length);


void udp_server_init(void) {
	struct udp_pcb *upcb;
	err_t err;

	/* Create a new UDP control block  */
	upcb = udp_new();

	if (upcb)
	{
	 /* Bind the upcb to the UDP_PORT port */
	 /* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
	  err = udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT);

	  if(err == ERR_OK)
	  {

		/* Set a receive callback for the upcb */
		udp_recv(upcb, udp_server_receive_callback, NULL);
	  }
	  else
	  {
		udp_remove(upcb);
	  }
	}
}

void udp_server_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
	unsigned char *data;
	unsigned short crc;
	unsigned short i=0;
	//unsigned char res = 0;
	static double position = 0;

	data = (unsigned char*)(p->payload);
	crc = GetCRC16(data,p->len);

	free_mem[6] = uxTaskGetStackHighWaterMark( NULL );

	if(crc==0)
	{
	  udp_tmr = 0;
	  reqID = (unsigned short)data[0]<<8;
	  reqID |= data[1];
	  switch(data[2]){
	  	  case 0xD2:
	  		  answer[0] = data[0];
	  		  answer[1] = data[1];
	  		  answer[2] = 0xD2;
	  		  // data[3] - address, data[4] - byte_count
	  		  for(i=0;i<data[4];i++) {
	  			  switch(data[3]+i) {
	  			  	  case 0:answer[3+i]=read_ip_from_conf(1);break;
	  			  	  case 1:answer[3+i]=read_ip_from_conf(2);break;
	  			  	  case 2:answer[3+i]=read_ip_from_conf(3);break;
	  			  	  case 3:answer[3+i]=read_ip_from_conf(4);break;
	  			  	  case 4:answer[3+i]=read_mask_from_conf(1);break;
	  			  	  case 5:answer[3+i]=read_mask_from_conf(2);break;
					  case 6:answer[3+i]=read_mask_from_conf(3);break;
					  case 7:answer[3+i]=read_mask_from_conf(4);break;
					  case 8:answer[3+i]=read_gate_from_conf(1);break;
					  case 9:answer[3+i]=read_gate_from_conf(2);break;
					  case 10:answer[3+i]=read_gate_from_conf(3);break;
					  case 11:answer[3+i]=read_gate_from_conf(4);break;
					  case 12:
						  position = (double)bl_num*100/1023 + 0.5;
						  answer[3+i]=(unsigned char)position;
						  break;
					  case 13:
						  answer[3+i] = bad_cnt;
						  break;
					  default:answer[3+i]=0;break;
	  			  }
	  		  }
	  		  crc = GetCRC16((unsigned char*)answer,3+data[4]);
			  answer[3+data[4]]=crc>>8;
			  answer[4+data[4]]=crc&0xFF;
			  send_udp_data(upcb, addr, port,5+data[4]);
	  		  break;
	  	  case 0xE2:
	  		answer[0] = data[0];
	  		answer[1] = data[1];
	  		answer[2] = 0xE2;
	  		/*for(i=0;i<data[4];i++) {
			  switch(data[3]+i) {
			  	  case 0:conf_ip[0]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, conf_ip[0]);break;
				  case 1:conf_ip[1]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, conf_ip[1]);break;
				  case 2:conf_ip[2]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, conf_ip[2]);break;
				  case 3:conf_ip[3]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, conf_ip[3]);break;
				  case 4:conf_mask[0]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR8, conf_mask[0]);break;
				  case 5:conf_mask[1]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR9, conf_mask[1]);break;
				  case 6:conf_mask[2]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR10, conf_mask[2]);break;
				  case 7:conf_mask[3]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR11, conf_mask[3]);break;
				  case 8:conf_gate[0]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR12, conf_gate[0]);break;
				  case 9:conf_gate[1]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR13, conf_gate[1]);break;
				  case 10:conf_gate[2]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR14, conf_gate[2]);break;
				  case 11:conf_gate[3]=data[5+i];HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR15, conf_gate[3]);break;
			  }
	  		}*/
	  		crc = GetCRC16((unsigned char*)answer,3);
		    answer[3]=crc>>8;
		    answer[4]=crc&0xFF;
		    send_udp_data(upcb, addr, port,5);
	  		break;
		  case 0xA0:
			  answer[0] = data[0];
			  answer[1] = data[1];
			  answer[2] = 0xA0;
			  answer[3] = 0x01;	// type of device identificator
			  answer[4] = 0xAD;
			  answer[5] = 0x54;
			  answer[6] = 0x98;
			  crc = GetCRC16((unsigned char*)answer,7);
			  answer[7]=crc>>8;
			  answer[8]=crc&0xFF;
			  send_udp_data(upcb, addr, port,9);
			  break;
		  /*case 0xA1:
			  reset_flash();
			  answer[0] = data[0];
			  answer[1] = data[1];
			  answer[2] = 0xA1;
			  crc = GetCRC16((unsigned char*)answer,3);
			  answer[3]=crc>>8;
			  answer[4]=crc&0xFF;
			  send_udp_data(upcb, addr, port,5);
			  break;
		  case 0xA2:
			  for(i=0;i<1024;i++) {
				res = erase_block(i);
				if(res==FLASH_ERROR) {
					add_bad_block(i);
					mark_bad_block(i);
				}
			  }
			  answer[0] = data[0];
			  answer[1] = data[1];
			  answer[2] = 0xA2;
			  crc = GetCRC16((unsigned char*)answer,3);
			  answer[3]=crc>>8;
			  answer[4]=crc&0xFF;
			  send_udp_data(upcb, addr, port,5);
			  break;

		  case 0xE8:
			  if(data[3]==4) {
				  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR19, 0x0000);
				  answer[0] = data[0];
				  answer[1] = data[1];
				  answer[2] = 0xE8;
				  answer[3] = 0x01;
				  crc = GetCRC16((unsigned char*)answer,4);
				  answer[4]=crc>>8;
				  answer[5]=crc&0xFF;
				  send_udp_data(upcb, addr, port,6);
			  }
			  else {
				  answer[0] = data[0];
				  answer[1] = data[1];
				  answer[2] = 0xE8;
				  answer[3] = 0x00;
				  crc = GetCRC16((unsigned char*)answer,4);
				  answer[4]=crc>>8;
				  answer[5]=crc&0xFF;
				  send_udp_data(upcb, addr, port,6);
			  }
			  break;
		  case 0xEF:
			  SCB->AIRCR = 0x05FA0004;
			  break;*/
		  case 0xD0:
			  if(isFlashBusy()) {
				  busy_cnt++;
				  answer[0] = data[0];
				  answer[1] = data[1];
				  answer[2] = 0xD0;
				  answer[3] = PAGE_BUSY;
				  crc = GetCRC16((unsigned char*)answer,4);
				  answer[4]=crc>>8;
				  answer[5]=crc&0xFF;
				  send_udp_data(upcb, addr, port,6);
				  break;
			  }
			  pcPageNum = (unsigned long)data[3]<<24;
			  pcPageNum |= (unsigned long)data[4]<<16;
			  pcPageNum |= (unsigned long)data[5]<<8;
			  pcPageNum |= data[6];

			  if((cache.page_num != (pcPageNum>>1)) || cache.ready==0) {
				  cache.page_num = pcPageNum>>1;
				  int_bl_num = cache.page_num / 64;
				  int_page_num = cache.page_num % 64;
				  cache.ready = read_page(int_bl_num,int_page_num,cache.buf);
			  }

			  // repeat request ID
			  answer[0] = data[0];
			  answer[1] = data[1];
			  answer[2] = 0xD0;

			  if(check_block(int_bl_num)==0) {
				  answer[3] = BAD_BLOCK;
				  crc = GetCRC16((unsigned char*)answer,4);
				  answer[4]=crc>>8;
				  answer[5]=crc&0xFF;
				  send_udp_data(upcb, addr, port,6);
			  }else {
				  if(int_bl_num<1024) {
					  if(cache.ready==FLASH_OK)
					  {

						  busy_cnt=0;
						  switch(pcPageNum%2) {
							  case 0:cache_offset=0;break;
							  case 1:cache_offset=1024;break;
						  }
						  //if(cache.buf[cache_offset+5]!=0xFF) HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
						  //for(i=0;i<1024;i++) answer[3+i] = cache.buf[cache_offset+i];
						  memcpy((unsigned char*)&answer[3],&cache.buf[cache_offset],1024);
						  crc = GetCRC16((unsigned char*)answer,3 + 1024);
						  answer[3+1024]=crc>>8;
						  answer[4+1024]=crc&0xFF;
						  send_udp_data(upcb, addr,port,5+1024);
					  }else {
						  if(cache.ready==FLASH_ERROR) {busy_cnt++;reset_flash();answer[3]=READ_ERROR;}
						  else {busy_cnt++;reset_flash();answer[3] = PAGE_BUSY;}
						  crc = GetCRC16((unsigned char*)answer,4);
						  answer[4]=crc>>8;
						  answer[5]=crc&0xFF;
						  send_udp_data(upcb, addr, port,6);
					  }
				  }else {
					  answer[3] = INCORRECT_PAGE_NUM;
					  crc = GetCRC16((unsigned char*)answer,4);
					  answer[4]=crc>>8;
					  answer[5]=crc&0xFF;
					  send_udp_data(upcb, addr, port,6);
				  }
			  }



			  break;
		  case 0xD1:

			  curTime = cTime;//EncodeDateTime(&sdt);

			  // repeat request ID
			  answer[0] = data[0];
			  answer[1] = data[1];
			  answer[2] = 0xD1;
			  answer[3] = (curTime>>24)&0xFF;
			  answer[4] = (curTime>>16)&0xFF;
			  answer[5] = (curTime>>8)&0xFF;
			  answer[6] = curTime & 0xFF;
			  for(i=0;i<12;i++) answer[7+i] = UniqueID[i];
			  answer[19] = 0x01; // version high
			  answer[20] = 0x00; // version low

			  crc = GetCRC16((unsigned char*)answer,7+12+2);
			  answer[21]=crc>>8;
			  answer[22]=crc&0xFF;
			  send_udp_data(upcb, addr, port,23);
			  break;
		  case 0xE1:
			  sTime.Seconds = data[3];
			  sTime.Minutes = data[4];
			  sTime.Hours = data[5];
			  sDate.Date = data[6];
			  sDate.Month = data[7];
			  sDate.Year = data[8];
			  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
			  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

			  counterFromTime();

			  answer[0] = data[0];
			  answer[1] = data[1];
			  answer[2] = 0xE1;
			  crc = GetCRC16((unsigned char*)answer,3);
			  answer[3]=crc>>8;
			  answer[4]=crc&0xFF;
			  send_udp_data(upcb, addr, port,5);
	  }
	}

	pbuf_free(p);
}

void inline send_udp_data(struct udp_pcb *upcb,const ip_addr_t *addr,u16_t port,u16_t length) {
	struct pbuf *p_answer;
	udp_connect(upcb, addr, port);
	p_answer = pbuf_alloc(PBUF_TRANSPORT,length, PBUF_POOL);
	if (p_answer != NULL)
	{
	  pbuf_take(p_answer, answer, length);
	  udp_send(upcb, p_answer);
	  pbuf_free(p_answer);
	}
	udp_disconnect(upcb);
}
