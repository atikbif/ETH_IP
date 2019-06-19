/*
 * http_server.c
 *
 *  Created on: 28 мая 2019 г.
 *      Author: User
 */


#include "lwip.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "main.h"
#include "lwip/apps/fs.h"
#include <string.h>
#include "canViewer.h"

extern uint8_t conf[64];

extern void read_conf();
extern void write_conf();

//uint8_t reset_flag = 0;

static const unsigned char PAGE_HEADER_200_OK[] = {
  //"HTTP/1.0 200 OK"
  0x48,0x54,0x54,0x50,0x2f,0x31,0x2e,0x31,0x20,0x32,0x30,0x30,0x20,0x4f,0x4b,0x0d,
  0x0a,
  //zero
  0x00
};

static char PAGE_BODY[3512] ;
uint16_t len = 0;

static const unsigned char PAGE_HEADER_SERVER[] = {
  //"Server: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)"
  0x53,0x65,0x72,0x76,0x65,0x72,0x3a,0x20,0x6c,0x77,0x49,0x50,0x2f,0x31,0x2e,0x33,
  0x2e,0x31,0x20,0x28,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x73,0x61,0x76,0x61,0x6e,
  0x6e,0x61,0x68,0x2e,0x6e,0x6f,0x6e,0x67,0x6e,0x75,0x2e,0x6f,0x72,0x67,0x2f,0x70,
  0x72,0x6f,0x6a,0x65,0x63,0x74,0x73,0x2f,0x6c,0x77,0x69,0x70,0x29,0x0d,0x0a,
  //zero
  0x00
};
static const unsigned char PAGE_HEADER_CONTENT_TEXT[] = {
  //"Content-length: 45"
  //"Content-type: text/html"
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3a,0x20,
  0x34,0x35,0x0d,0x0a,
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x74,0x79,0x70,0x65,0x3a,0x20,0x74,0x65,
  0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x0d,0x0a,0x0d,0x0a,
  //zero
  0x00
};

static const unsigned char PAGE_HEADER_DYN_CONTENT_TEXT[] = {
  //"Content-length: 8"
  //"Content-type: text/html"
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3a,0x20,
  0x38,0x0d,0x0a,
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x74,0x79,0x70,0x65,0x3a,0x20,0x74,0x65,
  0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x0d,0x0a,0x0d,0x0a,
  //zero
  0x00
};

static const unsigned char PAGE_HEADER_CAN_CONTENT_TEXT[] = {
  //"Content-length: 3018"
  //"Content-type: text/html"
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x4C,0x65,0x6E,0x67,0x74,0x68,0x3a,0x20,
  0x33,0x30,0x31,0x38,0x0d,0x0a,
  0x43,0x6f,0x6e,0x74,0x65,0x6e,0x74,0x2d,0x74,0x79,0x70,0x65,0x3a,0x20,0x74,0x65,
  0x78,0x74,0x2f,0x68,0x74,0x6d,0x6c,0x0d,0x0a,0x0d,0x0a,
  //zero
  0x00
};

uint8_t dt,month,year,h,min,sec;
RTC_TimeTypeDef plc_time = {0};
RTC_DateTypeDef plc_date = {0};

extern uint8_t can_req_msg[CAN_REQ_CNT][100];
extern RTC_HandleTypeDef hrtc;

uint8_t dynamic_page[8];

#define HTTPSERVER_THREAD_PRIO  ( tskIDLE_PRIORITY + 3 )

extern void clear_can_msg(void);
extern void update_can_msg();
extern uint8_t print_plc_time(uint8_t *buf);

uint16_t add_conf_data(uint16_t offset) {
	read_conf();
	memcpy(&PAGE_BODY[offset],&conf[2], 9+12*3);
	return offset+9+12*3;
}

uint16_t add_can_data(uint16_t offset) {
	clear_can_msg();
	update_can_msg();
	offset+=print_plc_time((uint8_t*)&PAGE_BODY[offset]);
	memcpy(&PAGE_BODY[offset],&can_req_msg[0][0], 3000);
	return offset+3000;
}

uint16_t add_dyn_data(uint16_t offset) {
	memcpy(&PAGE_BODY[offset],dynamic_page, 8);
	return offset+8;
}


static uint8_t hex_table[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

extern RNG_HandleTypeDef hrng;

static void update_dynamic_page() {
	uint32_t v = HAL_RNG_GetRandomNumber(&hrng);
	dynamic_page[0] = hex_table[(v&0xFF)/16];
	dynamic_page[1] = hex_table[(v&0xFF)%16];
	dynamic_page[2] = hex_table[((v>>8)&0xFF)/16];
	dynamic_page[3] = hex_table[((v>>8)&0xFF)%16];
	dynamic_page[4] = hex_table[((v>>16)&0xFF)/16];
	dynamic_page[5] = hex_table[((v>>16)&0xFF)%16];
	dynamic_page[6] = hex_table[((v>>24)&0xFF)/16];
	dynamic_page[7] = hex_table[((v>>24)&0xFF)%16];
}

static void http_server_serve(struct netconn *conn)
{
	struct netbuf *inbuf;
	err_t recv_err;
	char* buf;
	u16_t buflen;
	struct fs_file file;

	recv_err = netconn_recv(conn, &inbuf);

	if (recv_err == ERR_OK)
	{
		if (netconn_err(conn) == ERR_OK)
		{
			do
			{
				netbuf_data(inbuf, (void**)&buf, &buflen);
				if ((buflen >=5) && (strncmp(buf, "GET /", 5) == 0))
				{
					if (strncmp((char const *)buf,"GET / ",6)==0)
					{
						update_dynamic_page();
						fs_open(&file, "/user.html");
						netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
						fs_close(&file);
					}else if(strncmp((char const *)buf,"GET /dynamic.html?login",11)==0) {
						if((strncmp((char const *)&buf[18],"login=admin",11)==0) && (strncmp((char const *)&buf[30],"password=",9)==0))
						{
							if(buf[39]==conf[47] && buf[40]==conf[48] && buf[41]==conf[49] && buf[42]==conf[50]) {
								update_dynamic_page();
								sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_DYN_CONTENT_TEXT);
								len = strlen(PAGE_BODY);
								len = add_dyn_data(len);
								netconn_write(conn, PAGE_BODY, len, NETCONN_NOCOPY);
							}else {
								fs_open(&file, "/404.html");
								netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
								fs_close(&file);
							}

						}else {
							fs_open(&file, "/404.html");
							netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
							fs_close(&file);
						}
					}
					else if(strncmp((char const *)buf,"GET /write.html?node=",21)==0) {
						sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CONTENT_TEXT);
						//len = strlen(PAGE_BODY);
						memcpy(&conf[4], &((uint8_t*)buf)[21], 7+12*3);
						write_conf();
						//len = add_conf_data(len);
						//netconn_write(conn, PAGE_BODY, len, NETCONN_NOCOPY);
					}else if(strncmp((char const *)buf,"GET /reset.html",15)==0) {
						sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CONTENT_TEXT);
						//len = strlen(PAGE_BODY);
						//len = add_conf_data(len);
						//netconn_write(conn, PAGE_BODY, len, NETCONN_NOCOPY);
						HAL_Delay(100);
						NVIC_SystemReset();
						//reset_flag = 1;
					}else if(strncmp((char const *)buf,"GET /read.html",14)==0) {
						sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CONTENT_TEXT);
						len = strlen(PAGE_BODY);
						len = add_conf_data(len);
						netconn_write(conn, PAGE_BODY, len, NETCONN_NOCOPY);
					}else if(strncmp((char const *)buf,"GET /can.html",13)==0) {
						sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_CAN_CONTENT_TEXT);
						len = strlen(PAGE_BODY);
						len = add_can_data(len);
						netconn_write(conn, PAGE_BODY, len, NETCONN_NOCOPY);
					}else if(strncmp((char const *)&buf[9],(const char*)dynamic_page,8)==0) {
						fs_open(&file, "/index.html");
						netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
						fs_close(&file);
					}else if(strncmp((char const *)buf,"GET /synchro.html?date=",23)==0) {
						dt = ((uint8_t)buf[23]-'0')*10 + ((uint8_t)buf[24]-'0');
						month = ((uint8_t)buf[25]-'0')*10 + ((uint8_t)buf[26]-'0');
						year = ((uint8_t)buf[27]-'0')*10 + ((uint8_t)buf[28]-'0');
						h = ((uint8_t)buf[29]-'0')*10 + ((uint8_t)buf[30]-'0');
						min = ((uint8_t)buf[31]-'0')*10 + ((uint8_t)buf[32]-'0');
						sec = ((uint8_t)buf[33]-'0')*10 + ((uint8_t)buf[34]-'0');
						//HAL_RTC_GetTime(&hrtc, &plc_time, RTC_FORMAT_BIN);
						//HAL_RTC_GetDate(&hrtc, &plc_date, RTC_FORMAT_BIN);
						plc_time.Hours = h;
						plc_time.Minutes = min;
						plc_time.Seconds = sec;
						plc_time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
						plc_time.StoreOperation = RTC_STOREOPERATION_RESET;
						plc_date.Date = dt;
						plc_date.Month = month;
						plc_date.Year = year;
						fs_open(&file, "/404.html");
						netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
						fs_close(&file);



						HAL_RTC_SetTime(&hrtc, &plc_time, RTC_FORMAT_BIN);
						HAL_RTC_SetDate(&hrtc, &plc_date, RTC_FORMAT_BIN);

					}else if(strncmp((char const *)buf,"GET /new_password.html",22)==0) {
						fs_open(&file, "/new_password.html");
						netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
						fs_close(&file);
					}else if(strncmp((char const *)buf,"GET /password_update.html?old_password=",39)==0) {
						if(buf[39]==conf[47] && buf[40]==conf[48] && buf[41]==conf[49] && buf[42]==conf[50]) {
							conf[47] = buf[39+18];
							conf[48] = buf[40+18];
							conf[49] = buf[41+18];
							conf[50] = buf[42+18];
							write_conf();
							sprintf(PAGE_BODY,"%s%s%s",PAGE_HEADER_200_OK,PAGE_HEADER_SERVER,PAGE_HEADER_DYN_CONTENT_TEXT);
							len = strlen(PAGE_BODY);
							len = add_dyn_data(len);
							netconn_write(conn, PAGE_BODY, len, NETCONN_NOCOPY);
							// 18 - length "XXXX&new_password="
						}else {
							fs_open(&file, "/404.html");
							netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
							fs_close(&file);
						}
					}
					else
					{
						fs_open(&file, "/404.html");
						netconn_write(conn, file.data, file.len, NETCONN_NOCOPY);
						fs_close(&file);
					}
				}
			}while (netbuf_next(inbuf) >= 0);

		}
	}
	netconn_close(conn);
	netbuf_delete(inbuf);
}

static void http_server_thread(void *arg)
{
	struct netconn *conn, *newconn;
	err_t err, accept_err;

	update_dynamic_page();

	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);

	if (conn!= NULL)
	{
	    /* Bind to port 80 (HTTP) with default IP address */
	    err = netconn_bind(conn, NULL, 80);

	    if (err == ERR_OK)
	    {
	      /* Put the connection into LISTEN state */
	      netconn_listen(conn);

	      while(1)
	      {
	        /* accept any icoming connection */
	        accept_err = netconn_accept(conn, &newconn);
	        if(accept_err == ERR_OK)
	        {
	            /* serve connection */
	            http_server_serve(newconn);

	            /* delete connection */
	            netconn_delete(newconn);
	        }
	      }
	    }
	  }
}

void http_server_init(void)
{
  sys_thread_new("http_thread", http_server_thread, NULL, DEFAULT_THREAD_STACKSIZE*1, HTTPSERVER_THREAD_PRIO);
}
