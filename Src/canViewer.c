/*
 * canViewer.c
 *
 *  Created on: 22 апр. 2019 г.
 *      Author: User
 */


#include "canViewer.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#include "main.h"
#include "datetime.h"
#include "crc.h"
#include "canLogger.h"

#define VIEW_TIME	3000
#define UPD_TIME	10000

extern CAN_HandleTypeDef hcan1;
extern RNG_HandleTypeDef hrng;
static CAN_RxHeaderTypeDef   RxHeader;
static uint8_t               RxData[8];
static CAN_TxHeaderTypeDef   TxHeader;
static uint32_t              TxMailbox=0;
static uint8_t               TxData[8];
uint16_t can_tmr=0;

extern uint8_t answer_90[246];
extern uint8_t answer_91[28];
extern uint8_t answer_92[14];
extern uint8_t answer_93[14];
extern uint8_t answer_94[28];
extern uint8_t answer_95[128];
extern uint8_t answer_96[16];
extern uint8_t answer_97[128];
extern uint8_t answer_98[128];
extern uint8_t answer_99[4];
extern uint8_t answer_9a[34];
uint8_t answer_9b[2];
extern uint8_t answer_9c[74];
extern uint8_t answer_time[4];

extern uint8_t ob_99_upd;
extern uint8_t ob_9a_upd;
extern uint8_t ob_time_upd;

extern uint8_t eth_ip_state;
extern uint16_t eth_ip_tmr;

extern uint8_t conf[64];

static uint16_t node_tmr[7] = {0,0,0,0,0,0,0};

static uint16_t clusters_tmr = 0;
static uint8_t clusters_update = 0;
static uint8_t clusters_state = 0;
static unsigned short remainBytes = 2048;

static struct logBuf* log;

uint16_t debug_cnt = 0;

#define CAN_TEST_LENGTH	9


#ifdef DEBUG_MODE
static uint8_t can_test_data[CAN_TEST_LENGTH][11] = {
		{0x04,0x07,0x08,0x0F,0x1C,0x64,0xBC,0x00,0x00,0x00,0x00},	// 2 bytes - id, 1 byte - data length, 8 bytes - data
		{0x04,0x07,0x08,0x0F,0x1E,0xC8,0x78,0x00,0x00,0x00,0x00},
		{0x04,0x07,0x08,0x0F,0x1E,0x67,0x41,0x00,0x00,0x00,0x00},
		{0x04,0x07,0x08,0x0F,0x1E,0x04,0x8E,0x00,0x00,0x00,0x00},
		{0x04,0x07,0x08,0x0F,0x1E,0x01,0x6D,0x00,0x00,0x00,0x00},
		{0x04,0x07,0x08,0x0F,0x1E,0x02,0x47,0x00,0x00,0x00,0x00},
		{0x04,0x01,0x02,0x09,0x58,0x00,0x00,0x00,0x00,0x00,0x00},
		{0x04,0x19,0x02,0x09,0xF1,0x00,0x00,0x00,0x00,0x00,0x00},
		{0x04,0x09,0x02,0x09,0x9F,0x00,0x00,0x00,0x00,0x00,0x00},
};
#endif

static unsigned long extTime = 0;

extern unsigned long cTime;

typedef struct{
	uint8_t addr;
	uint8_t service;
	uint8_t ss;
	uint8_t eoid;
	uint8_t intern_addr;
	uint8_t data[8];
	uint8_t data_length;
	uint8_t sec;
	uint8_t min;
	uint8_t houre;
	uint8_t active;
}can_req;
can_req can_reqs[CAN_REQ_CNT];

uint8_t can_req_msg[CAN_REQ_CNT][100];

extern RTC_HandleTypeDef hrtc;

RTC_TimeTypeDef time;
RTC_DateTypeDef date;


uint8_t can_pos = 0;

static void add_can_request(can_req *req) {
	can_reqs[can_pos] = *req;
	can_reqs[can_pos].active = 1;
	can_pos++;if(can_pos>=CAN_REQ_CNT) can_pos = 0;
}

static uint8_t hex_table[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static unsigned char crc_buf[64];
static uint8_t debug_num = 0;

static void print_hex(uint8_t *buf, uint8_t value) {
	buf[0] = '0';
	buf[1] = 'x';
	buf[2] = hex_table[value/16];
	buf[3] = hex_table[value%16];
	buf[4] = ' ';
}

static void print_dec(uint8_t *buf, uint8_t value) {
	if(value>99) value = 99;
	buf[0] = value/10 + '0';
	buf[1] = value%10 + '0';
}

static void print_time(uint8_t *buf, can_req *req) {
	print_dec(&buf[0],req->houre);
	buf[2]=':';
	print_dec(&buf[3],req->min);
	buf[5]=':';
	print_dec(&buf[6],req->sec);
	buf[8]=' ';
}

uint8_t print_plc_time(uint8_t *buf) {
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	print_dec(&buf[0],date.Date);
	buf[2] = '/';
	print_dec(&buf[3],date.Month);
	buf[5] = '/';
	print_dec(&buf[6],date.Year);
	buf[8] = ' ';
	print_dec(&buf[9],time.Hours);
	buf[11]=':';
	print_dec(&buf[12],time.Minutes);
	buf[14]=':';
	print_dec(&buf[15],time.Seconds);
	buf[17]=' ';
	return 18;
}

static void print_message(uint8_t *buf, can_req *req) {
	uint8_t i = 0, j = 0;
	print_time(&buf[0],req);
	buf[9] = 'N';buf[10]='O';buf[11]='D';buf[12]='E';buf[13]=':';
	print_hex(&buf[14],req->addr);
	if(req->service==0x01) {
		buf[19] = 'H';
		buf[20] = 'E';
		buf[21] = 'A';
		buf[22] = 'R';
		buf[23] = 'T';
		buf[24] = 'B';
		buf[25] = 'E';
		buf[26] = 'A';
		buf[27] = 'T';
		buf[28] = ':';
		print_hex(&buf[29],req->intern_addr);
	}else {
		buf[19] = 'S';
		buf[20] = 'R';
		buf[21] = 'V';
		buf[22] = ':';
		print_hex(&buf[23],req->service);
		buf[28] = 'S';
		buf[29] = 'S';
		buf[30] = ':';
		print_hex(&buf[31],req->ss);
		buf[36] = 'I';
		buf[37] = 'D';
		buf[38] = ':';
		print_hex(&buf[39],req->eoid);
		buf[44]='A';buf[45]='D';buf[46]='D';buf[47]='R';buf[48]=':';
		print_hex(&buf[49],req->intern_addr);
		buf[54] = 'D';
		buf[55] = 'A';
		buf[56] = 'T';
		buf[57] = 'A';
		buf[58] = ':';

		j = req->data_length;
		if(j>8) j = 8;
		for(i=0;i<j;i++) {
			print_hex(&buf[59+5*i],req->data[i]);
		}
	}
	buf[99] = 0x0D;

	//buf[53+5*i] = 0x0d;
}

void clear_can_msg(void) {
	uint8_t i,j;
	for(i=0;i<CAN_REQ_CNT;i++) {
		for(j=0;j<99;j++) can_req_msg[i][j] = ' ';
		can_req_msg[i][99] = 0x0d;
	}
}

void update_can_msg() {
	uint8_t i,j;
	uint8_t pos = 0;
	if(can_pos) pos = can_pos-1;else pos = CAN_REQ_CNT - 1;
	for(i=0;i<CAN_REQ_CNT;i++) {
		if(can_reqs[pos].active) {
			print_message(can_req_msg[CAN_REQ_CNT-1-i],&can_reqs[pos]);

		}else {
			for(j=0;j<99;j++) can_req_msg[CAN_REQ_CNT-1-i][j]=' ';
			can_req_msg[i][99] = 0x0d;
		}
		if(pos) pos--;else pos = CAN_REQ_CNT - 1;
	}
}


static void initCANViewerFilter() {
	CAN_FilterTypeDef  sFilterConfig;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;
	HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
}

static void info_net_work(uint16_t *tmr) {
	(*tmr)++;
	if((*tmr)==VIEW_TIME) {	// TELEMETRY ERRORS
		TxHeader.StdId = 0x400 | 0x3F;
		TxHeader.ExtId = 0;
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.IDE = CAN_ID_STD;
		TxHeader.DLC = 3;
		TxHeader.TransmitGlobalTime = DISABLE;
		TxData[0] = 0x1F;
		TxData[1] = 0x0d;
		if(eth_ip_state==0x00) TxData[2] = 0x02;
		else if(eth_ip_state==0x01) TxData[1] = 0x04;
		else if(eth_ip_state==0x02) TxData[1] = 0x01;
		else TxData[2] = 0x02;
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	}else if((*tmr)==VIEW_TIME+50) {	// COMMUNICATION STATUS
		TxHeader.StdId = 0x400 | 0x3F;
		TxHeader.ExtId = 0;
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.IDE = CAN_ID_STD;
		TxHeader.DLC = 5;
		TxHeader.TransmitGlobalTime = DISABLE;
		TxData[0] = 0x1F;
		TxData[1] = 0x11;
		TxData[2] = eth_ip_state;
		TxData[3] = 0x00;
		TxData[4] = 0x00;
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	}else if((*tmr)==VIEW_TIME+100) {	// display tx mask (node-1, switch input)
		/*TxHeader.StdId = 0x0E;
		TxHeader.ExtId = 0;
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.IDE = CAN_ID_STD;
		TxHeader.DLC = 5;
		TxHeader.TransmitGlobalTime = DISABLE;
		TxData[0] = 0x0A;
		TxData[1] = 0x00;
		TxData[2] = 0xFF;
		TxData[3] = 0x00;
		TxData[4] = 0x02;
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);*/
	}else if((*tmr)>VIEW_TIME+100) {(*tmr) = 0;}
}

static void telemetry_work(uint16_t *tmr) {
	static uint8_t node_num = 0;
	static uint8_t req_num = 0;
	static uint8_t start_flag = 0;
	static uint16_t req_period = 0;
	(*tmr)++;
	if((*tmr)==UPD_TIME) {(*tmr)=0;start_flag = 1;req_period=50;node_num=0;req_num=0;}	// начало цикла
	if(start_flag) {
		if(req_period>=50) {
			req_period = 0;
			if(node_num==7) {
				if(req_num==0) {
					TxHeader.StdId = 0x400 | 0x3F;
					TxHeader.ExtId = 0;
					TxHeader.RTR = CAN_RTR_DATA;
					TxHeader.IDE = CAN_ID_STD;
					TxHeader.DLC = 3;
					TxHeader.TransmitGlobalTime = DISABLE;
					TxData[0] = 0x1F;
					TxData[1] = 0x00;
					TxData[2] = 0x0b;
					HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
					req_num = 1;
				}else if(req_num==1) {
					TxHeader.StdId = 0x400 | 0x3F;
					TxHeader.ExtId = 0;
					TxHeader.RTR = CAN_RTR_DATA;
					TxHeader.IDE = CAN_ID_STD;
					TxHeader.DLC = 4;
					TxHeader.TransmitGlobalTime = DISABLE;
					TxData[0] = 0x1F;
					TxData[1] = 0x03;
					TxData[2] = 0x8F;
					TxData[3] = 0x13;
					HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
					node_num=0;
					req_num = 0;
					start_flag = 0;	// завершение цикла
				}else {
					node_num = 0;
					req_num = 0;
					start_flag = 0;	// завершение цикла
				}
			}else {
				switch(conf[4+node_num]) {
				case '0':	// нет узла
					req_num = 0;
					while(node_num<7) { // пропуск пустых узлов
						node_num++;
						if(node_num<7 && conf[4+node_num!='0']) break;
					}
					break;
				case '1':	// PC21-1
					if(req_num==0) {	// digital inp
						TxHeader.StdId = 0x06 | (node_num<<3);
						TxHeader.ExtId = 0;
						TxHeader.RTR = CAN_RTR_DATA;
						TxHeader.IDE = CAN_ID_STD;
						TxHeader.DLC = 5;
						TxHeader.TransmitGlobalTime = DISABLE;
						TxData[0] = 0x0A;
						TxData[1] = 0x01;
						TxData[2] = 0xFF;
						TxData[3] = 0x3F;
						TxData[4] = 0x01;
						HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
						req_num = 1;
					}else if(req_num==1) {	// digital out
						TxHeader.StdId = 0x06 | (node_num<<3);
						TxHeader.ExtId = 0;
						TxHeader.RTR = CAN_RTR_DATA;
						TxHeader.IDE = CAN_ID_STD;
						TxHeader.DLC = 5;
						TxHeader.TransmitGlobalTime = DISABLE;
						TxData[0] = 0x0A;
						TxData[1] = 0x01;
						TxData[2] = 0xFF;
						TxData[3] = 0x3F;
						TxData[4] = 0x03;
						HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
						req_num = 2;
					}else if(req_num==2) {	// analogue inp
						TxHeader.StdId = 0x06 | (node_num<<3);
						TxHeader.ExtId = 0;
						TxHeader.RTR = CAN_RTR_DATA;
						TxHeader.IDE = CAN_ID_STD;
						TxHeader.DLC = 5;
						TxHeader.TransmitGlobalTime = DISABLE;
						TxData[0] = 0x0A;
						TxData[1] = 0x01;
						TxData[2] = 0xFF;
						TxData[3] = 0x3F;
						TxData[4] = 0x00;
						HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
						req_num = 0;
						node_num++;
					}else {
						req_num = 0;
						node_num++;
					}
					break;
				case '2':	// PC21-CD
					if(req_num==0) {
						TxHeader.StdId = 0x06 | (node_num<<3);
						TxHeader.ExtId = 0;
						TxHeader.RTR = CAN_RTR_DATA;
						TxHeader.IDE = CAN_ID_STD;
						TxHeader.DLC = 5;
						TxHeader.TransmitGlobalTime = DISABLE;
						TxData[0] = 0x0A;
						TxData[1] = 0x01;
						TxData[2] = 0xFF;
						TxData[3] = 0x00;
						TxData[4] = 0x02;
						HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
						req_num = 0;
						node_num++;
					}else {
						req_num = 0;
						node_num++;
					}
					break;
				case '3':	// PC21-MC
					if(req_num==0) {	// digital inp
						TxHeader.StdId = 0x06 | (node_num<<3);
						TxHeader.ExtId = 0;
						TxHeader.RTR = CAN_RTR_DATA;
						TxHeader.IDE = CAN_ID_STD;
						TxHeader.DLC = 5;
						TxHeader.TransmitGlobalTime = DISABLE;
						TxData[0] = 0x0A;
						TxData[1] = 0x01;
						TxData[2] = 0xFF;
						TxData[3] = 0x3F;
						TxData[4] = 0x01;
						HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
						req_num = 1;
					}else if(req_num==1) {	// digital out
						TxHeader.StdId = 0x06 | (node_num<<3);
						TxHeader.ExtId = 0;
						TxHeader.RTR = CAN_RTR_DATA;
						TxHeader.IDE = CAN_ID_STD;
						TxHeader.DLC = 5;
						TxHeader.TransmitGlobalTime = DISABLE;
						TxData[0] = 0x0A;
						TxData[1] = 0x01;
						TxData[2] = 0xFF;
						TxData[3] = 0x3F;
						TxData[4] = 0x03;
						HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
						req_num = 2;
					}else if(req_num==2) {	// analogue inp
						TxHeader.StdId = 0x06 | (node_num<<3);
						TxHeader.ExtId = 0;
						TxHeader.RTR = CAN_RTR_DATA;
						TxHeader.IDE = CAN_ID_STD;
						TxHeader.DLC = 5;
						TxHeader.TransmitGlobalTime = DISABLE;
						TxData[0] = 0x0A;
						TxData[1] = 0x01;
						TxData[2] = 0xFF;
						TxData[3] = 0x3F;
						TxData[4] = 0x00;
						HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
						req_num = 0;
						node_num++;
					}else {
						req_num = 0;
						node_num++;
					}
					break;
				default:
					node_num++;
				}
				if(node_num==7) req_num=0;
			}
		}else req_period++;
	}
}

static inline unsigned char writeByteToBuffer(unsigned char value) {
	if(remainBytes) {
		log->buf[2048-remainBytes] = value;
	}else {
		if(log->next->fillFlag==0) {
			log->fillFlag = 1;
			log = log->next;
			remainBytes = 2048;
			log->buf[0] = value;
		}else return 0;
	}
	remainBytes--;
	return 1;
}

static uint8_t get_free_buf_space(uint8_t value) {
	if((value<=remainBytes) || log->next->fillFlag==0) return 1;
	return 0;
}

void canViewerTask(void const * argument) {
	can_req cr;
	uint8_t eoid = 0;
	uint8_t node_addr = 0;
	//uint8_t serv = 0;
	uint8_t intern_addr = 0;
	//uint8_t ss = 0;
	uint16_t pos = 0;
	uint16_t i = 0;
	uint8_t byte_value = 0;
	uint16_t word_value = 0;
	uint8_t tmp = 0;
	initCANViewerFilter();
	HAL_CAN_Start(&hcan1);

	uint16_t heartbeat_cnt = 0;
	uint8_t heartbeat_value = 1;
	uint16_t info_tmr = VIEW_TIME-500;
	uint16_t telemetry_tmr = UPD_TIME-100;

	/*osDelay(100);
	TxHeader.StdId = 0x06;
	TxHeader.ExtId = 0;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 2;
	TxHeader.TransmitGlobalTime = DISABLE;
	TxData[0] = 0x1C;
	TxData[1] = 0x01;
	HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3) {osDelay(1);}

	osDelay(100);
	TxHeader.StdId = 0x0E;
	TxHeader.ExtId = 0;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 2;
	TxHeader.TransmitGlobalTime = DISABLE;
	TxData[0] = 0x1C;
	TxData[1] = 0x01;
	HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3) {osDelay(1);}*/

	HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);
	osDelay(1000);
	log = getFirstLog();

	for(;;)
	{
#ifdef DEBUG_MODE
		debug_cnt++;
		if(debug_cnt>=1){
			HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
			if(get_free_buf_space(1+2+4+1+can_test_data[debug_num][2]+1)) {
				writeByteToBuffer(0x31);
				crc_buf[0]=can_test_data[debug_num][0];writeByteToBuffer(crc_buf[0]);
				crc_buf[1]=can_test_data[debug_num][1];writeByteToBuffer(crc_buf[1]);

				crc_buf[2]=(cTime >> 24) & 0xFF;writeByteToBuffer(crc_buf[2]);
				crc_buf[3]=(cTime >> 16) & 0xFF;writeByteToBuffer(crc_buf[3]);
				crc_buf[4]=(cTime >> 8) & 0xFF;writeByteToBuffer(crc_buf[4]);
				crc_buf[5]=cTime & 0xFF;writeByteToBuffer(crc_buf[5]);

				crc_buf[6]=can_test_data[debug_num][2];writeByteToBuffer(crc_buf[6]);
				for(i=0;i<crc_buf[6];i++) {
					crc_buf[7+i]=can_test_data[debug_num][3+i];
					writeByteToBuffer(crc_buf[7+i]);
				}
				writeByteToBuffer(GetCRC8(crc_buf,7+i));

				cr.sec = time.Seconds;
				cr.min = time.Minutes;
				cr.houre = time.Hours;
				cr.eoid = can_test_data[debug_num][3] & 0x1F;
				cr.ss = can_test_data[debug_num][3] >> 5;
				//uint32_t rv = crc_buf[6];
				cr.data_length = crc_buf[6];
				cr.service = can_test_data[debug_num][1] & 0x07;
				cr.addr = (can_test_data[debug_num][1] >> 3) & 0x0F;
				cr.intern_addr = can_test_data[debug_num][4];
				for(i=0;i<cr.data_length;i++) {
					cr.data[i] = can_test_data[debug_num][5+i];
				}
				add_can_request(&cr);

				clear_can_msg();
				update_can_msg();

				debug_num++;
				if(debug_num>=CAN_TEST_LENGTH) debug_num = 0;
				//HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
			}
			debug_cnt = 0;



			//can_tmr=0;
		}
#endif
		if((answer_9b[0] & 0x7F)==0) {
			answer_9b[1] = 0;
			for(i=0;i<8;i++) {
				if(i<4) {
					for(tmp=0;tmp<32;tmp++)	answer_97[i*32+tmp]=0;
				}else {
					for(tmp=0;tmp<32;tmp++)	answer_98[(i-4)*32+tmp]=0;
				}
				answer_96[i*2] = 0;
				answer_96[i*2+1] = 0;
			}
		}
		eth_ip_tmr++;
		if(eth_ip_tmr>=3000) {
			eth_ip_tmr = 0;
			eth_ip_state = 0;
		}
		for(i=0;i<7;i++) {
			node_tmr[i]++;
			if(node_tmr[i]>=3000) {
				node_tmr[i] = 0;
				tmp = 1<<i;
				answer_9b[0] = answer_9b[0] & (~tmp);
			}
		}
		if(ob_time_upd) {
			ob_time_upd = 0;
			TxHeader.StdId = 0x400 | 0x3F;
			TxHeader.ExtId = 0;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 6;
			TxHeader.TransmitGlobalTime = DISABLE;
			TxData[0] = 0x1F;
			TxData[1] = 0x0E;
			TxData[2] = answer_time[0];
			TxData[3] = answer_time[1];
			TxData[4] = answer_time[2];
			TxData[5] = answer_time[3];
			HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
		}
		if(ob_99_upd) {
			ob_99_upd = 0;
			TxHeader.StdId = 0x400 | 0x3F;
			TxHeader.ExtId = 0;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 4;
			TxHeader.TransmitGlobalTime = DISABLE;
			TxData[0] = 0x1D;
			TxData[1] = 0x00;
			TxData[2] = answer_99[0];
			TxData[3] = answer_99[1];
			HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
		}
		if(ob_9a_upd) {
			TxHeader.StdId = 0x400 | 0x3F;
			TxHeader.ExtId = 0;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 4;
			TxHeader.TransmitGlobalTime = DISABLE;
			TxData[0] = 0x1E;
			TxData[1] = ob_9a_upd;
			TxData[2] = answer_9a[(ob_9a_upd-1)*2];
			TxData[3] = answer_9a[(ob_9a_upd-1)*2+1];
			HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
			while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3) {osDelay(1);}
			ob_9a_upd++;
			if(ob_9a_upd>=17) ob_9a_upd = 0;
		}
		telemetry_work(&telemetry_tmr);
		info_net_work(&info_tmr);
		heartbeat_cnt++;
		if(heartbeat_cnt>=250) {
			heartbeat_cnt = 0;
			TxHeader.StdId = 0x439;
			TxHeader.ExtId = 0;
			TxHeader.RTR = CAN_RTR_DATA;
			TxHeader.IDE = CAN_ID_STD;
			TxHeader.DLC = 2;
			TxHeader.TransmitGlobalTime = DISABLE;
			TxData[0] = 0x49;
			TxData[1] = heartbeat_value++;
			HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
		}
		if(HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0)) {
			if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
				can_tmr = 0;
				eoid = RxData[0] & 0x1F;
				//ss = RxData[0] >> 5;
				node_addr = (RxHeader.StdId>>3) & 0x7F;
				//serv = RxHeader.StdId & 0x07;
				intern_addr = RxData[1];

				if((eoid==0x1F)&&(RxData[1]==0x0E)&&(RxHeader.DLC==6)) {
					extTime = RxData[5];
					extTime<<=8;extTime|=RxData[4];
					extTime<<=8;extTime|=RxData[3];
					extTime<<=8;extTime|=RxData[2];
					updateCurrentTime(extTime);
				}

				if(get_free_buf_space(1+2+4+1+(RxHeader.DLC & 0xFF) + 1)) {
					writeByteToBuffer(0x31);
					crc_buf[0]=RxHeader.StdId >> 8;writeByteToBuffer(crc_buf[0]);
					crc_buf[1]=RxHeader.StdId & 0xFF;writeByteToBuffer(crc_buf[1]);

					crc_buf[2]=(cTime >> 24) & 0xFF;writeByteToBuffer(crc_buf[2]);
					crc_buf[3]=(cTime >> 16) & 0xFF;writeByteToBuffer(crc_buf[3]);
					crc_buf[4]=(cTime >> 8) & 0xFF;writeByteToBuffer(crc_buf[4]);
					crc_buf[5]=cTime & 0xFF;writeByteToBuffer(crc_buf[5]);

					crc_buf[6]=RxHeader.DLC & 0xFF;writeByteToBuffer(crc_buf[6]);
					for(i=0;i<crc_buf[6];i++) {
						crc_buf[7+i]=RxData[i];
						writeByteToBuffer(crc_buf[7+i]);
					}
					writeByteToBuffer(GetCRC8(crc_buf,7+i));
				}


				cr.service = RxHeader.StdId & 0x07;
				cr.addr = (RxHeader.StdId>>3) & 0x0F;
				HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
				cr.sec = time.Seconds;
				cr.min = time.Minutes;
				cr.houre = time.Hours;
				cr.ss = (RxData[0] >> 5) & 0x07;
				cr.eoid = eoid;
				cr.intern_addr = intern_addr;
				cr.data_length = RxHeader.DLC - 2;
				for(i=0;i<cr.data_length;i++) cr.data[i] = RxData[2+i];
				add_can_request(&cr);
				switch(eoid) {
				case 0x01:	// packed physical digits
					if(intern_addr>0 && intern_addr<=16) {	// digital inputs
						for(i=0;i<8;i++) {
							// i - bit number
							// check mask
							if(RxData[4]&(1<<i)) {
								pos = 4*(node_addr&0x0F) + ((intern_addr-1+i)/4);
								tmp = answer_91[pos]; // temporary buffer
								byte_value = 0x03;
								byte_value = byte_value << ((intern_addr-1+i)%4)*2;
								byte_value = ~byte_value;
								tmp=tmp & byte_value; // clear value position
								// check fault
								if(RxData[3]&(1<<i)) {
									byte_value = 0x03;	// fault code
									byte_value = byte_value << ((intern_addr-1+i)%4)*2;	// offset
									tmp|=byte_value;
									if(pos<sizeof(answer_91)) answer_91[pos] = tmp;
								}else {	// correct value
									if(RxData[2]&(1<<i)) byte_value = 0x00;
									else byte_value = 0x01;
									byte_value = byte_value << ((intern_addr-1+i)%4)*2;	// offset
									tmp|=byte_value;
									if(pos<sizeof(answer_91)) answer_91[pos] = tmp;
								}
							}
						}
						//answer_91[0] = RxHeader.DLC;
						//for(i=0;i<RxHeader.DLC;i++) answer_91[1+i] = RxData[i];
					}else if(intern_addr>=65 && intern_addr<=72) {	// switch inputs
						//answer_92[0] = 0x55;
						for(i=0;i<8;i++) {
							// i - bit number
							// check mask
							if(RxData[4]&(1<<i)) {
								pos = 2*(node_addr&0x0F) + ((intern_addr-65+i)/4);
								tmp = answer_92[pos]; // temporary buffer
								byte_value = 0x03;
								byte_value = byte_value << ((intern_addr-65+i)%4)*2;
								byte_value = ~byte_value;
								tmp=tmp & byte_value; // clear value position
								// check fault
								if(RxData[3]&(1<<i)) {
									byte_value = 0x03;	// fault code
									byte_value = byte_value << ((intern_addr-65+i)%4)*2;	// offset
									tmp|=byte_value;
									if(pos<sizeof(answer_92)) answer_92[pos] = tmp;
								}else {	// correct value
									if(RxData[2]&(1<<i)) byte_value = 0x00;
									else byte_value = 0x01;
									byte_value = byte_value << ((intern_addr-65+i)%4)*2;	// offset
									tmp|=byte_value;
									if(pos<sizeof(answer_92)) answer_92[pos] = tmp;
								}
							}
						}
					}else if(intern_addr>=129 && intern_addr<=136) {	// relay outputs
						for(i=0;i<8;i++) {
							// i - bit number
							// check mask
							if(RxData[4]&(1<<i)) {
								pos = 2*(node_addr&0x0F) + ((intern_addr-129+i)/4);
								tmp = answer_93[pos]; // temporary buffer
								byte_value = 0x03;
								byte_value = byte_value << ((intern_addr-129+i)%4)*2;
								byte_value = ~byte_value;
								tmp=tmp & byte_value; // clear value position
								// check fault
								if(RxData[3]&(1<<i)) {
									byte_value = 0x03;	// fault code
									byte_value = byte_value << ((intern_addr-129+i)%4)*2;	// offset
									tmp|=byte_value;
									if(pos<sizeof(answer_93)) answer_93[pos] = tmp;
								}else {	// correct value
									if(RxData[2]&(1<<i)) byte_value = 0x00;
									else byte_value = 0x01;
									byte_value = byte_value << ((intern_addr-129+i)%4)*2;	// offset
									tmp|=byte_value;
									if(pos<sizeof(answer_93)) answer_93[pos] = tmp;
								}
							}
						}
					}
					break;
				case 0x03:	// packed deduced digitals
					if(intern_addr>=16) {	// cluster global bits
						for(i=0;i<8;i++) {
							// check mask
							if(RxData[3]&(1<<i)) {
								pos = ((intern_addr-16)+i)/8;
								// bit number
								byte_value = ((intern_addr-16)+i)%8;
								if(RxData[2]&(1<<i)) {
									if(pos<sizeof(answer_94)) answer_94[pos]|=1<<byte_value;
								}else {
									if(pos<sizeof(answer_94)) answer_94[pos]&=~(1<<byte_value);
								}
							}
						}
					}
					break;
				case 0x05:	// analogue data scaled with status
					// analogue data
					if(intern_addr>0 && intern_addr<=15) {
						word_value = (((uint16_t)RxData[7])<<8) | RxData[6];
						pos = 32*(node_addr&0x0F) + (intern_addr-1)*2;
						//answer_90[pos++] = RxData[7];
						//answer_90[pos] = RxData[6];
						if(pos<sizeof(answer_90)) answer_90[pos] = RxData[2];
						pos++;
						//answer_90[pos] = RxData[3];
						if(pos<sizeof(answer_90)) {
							if(RxData[3]==0x01)
							{
								if(word_value<=200) {
									answer_90[pos] = 0xFD;
								}else if(word_value<=350) { // 350
									answer_90[pos] = 0xEF;
								}else if(word_value>=2200) {
									answer_90[pos] = 0xFD;
								}else if(word_value>=2050) {
									answer_90[pos] = 0xF7;
								}else answer_90[pos] = 0xFF;
								//if(RxData[4]) answer_90[pos] = 0xFD;
							}else if(RxData[3]==0x04)
							{
								if(word_value<=176*2) {
									answer_90[pos] = 0xFD;
								}else if(word_value<=176*3.5) { // 350
									answer_90[pos] = 0xEF;
								}else if(word_value>=176*22) {
									answer_90[pos] = 0xFD;
								}else if(word_value>=176*20.5) {
									answer_90[pos] = 0xF7;
								}else answer_90[pos] = 0xFF;
							}else {
								if(RxData[4]) answer_90[pos] = 0xFD;
								else answer_90[pos] = 0xFF;
							}
						}
					}
					break;
				case 0x06:	// global integer values
					if(intern_addr>=17 && intern_addr<=80) {
						pos = (intern_addr-17)*2;
						if(pos<sizeof(answer_95)) answer_95[pos++] = RxData[2];
						if(pos<sizeof(answer_95)) answer_95[pos] = RxData[3];
					}
					break;
				case 0x09:	// heartbeat count
					tmp = node_addr&0x0F;
					if(tmp<8) {
						node_tmr[tmp] = 0;
						answer_9b[0] |= 1<<tmp;
					}
					break;
				case 0x0A:	// ---
					break;
				case 0x0B:	// ---
					break;
				case 0x0C:	// cluster status
					/*tmp = node_addr>>4;
					if(tmp<8) {
						if(intern_addr==4) answer_9b[1]|=1<<intern_addr;
						else answer_9b[1]&=~(1<<intern_addr);
					}*/
					break;
				case 0x0D:	// network status
					if(RxData[2]!=answer_9b[1]) {
						clusters_tmr = 0;
						clusters_update = 1;
						clusters_state = RxData[2];
					}
					break;
				case 0x0E:	// network packed deduced digitals
					if(intern_addr>=1) {
						for(i=0;i<8;i++) {
							// check mask
							if(RxData[3]&(1<<i)) {
								pos = ((intern_addr-1)+i)/8;
								// bit number
								byte_value = ((intern_addr-1)+i)%8;
								if(RxData[2]&(1<<i)) {
									if(pos<sizeof(answer_96)) answer_96[pos]|=1<<byte_value;
								}else {
									if(pos<sizeof(answer_96)) answer_96[pos]&=~(1<<byte_value);
								}
							}
						}
					}

					break;
				case 0x0F:	// network integer values
					if(intern_addr>=1) {
						pos = (intern_addr-1)*2;
						if(pos<sizeof(answer_97)) {
							answer_97[pos++] = RxData[2];
							if(pos<sizeof(answer_97)) answer_97[pos] = RxData[3];
						}else {
							pos-=sizeof(answer_97);
							if(pos<sizeof(answer_98)) {
								answer_98[pos++] = RxData[2];
								if(pos<sizeof(answer_98)) answer_98[pos] = RxData[3];
							}
						}
					}
					break;
				case 0x10:	// ---
					break;
				case 0x11:	// reader message
					break;
				case 0x12:	// ---
					break;
				case 0x1D:	// telemetry controls
					if(intern_addr==0) {
						if(pos<sizeof(answer_99)) answer_99[0] = RxData[2];
						if(pos<sizeof(answer_99)) answer_99[1] = RxData[3];
					}
					break;
				case 0x1E:	// telemetry integers
					if(intern_addr>=0 && intern_addr<=7) {
						pos = intern_addr*2;
						if(pos<sizeof(answer_9a)) answer_9a[pos++] = RxData[2];
						if(pos<sizeof(answer_9a)) answer_9a[pos] = RxData[3];
					}
					break;
				case 0x1F:	// module data;
					if(intern_addr==18) {	// application checksum
						pos = 32*(node_addr&0x0F) + 30;
						if(pos<sizeof(answer_90)) answer_90[pos++] = RxData[2];
						if(pos<sizeof(answer_90)) answer_90[pos] = RxData[3];
					}else if(intern_addr==4) {	// Analog inputs
						word_value =RxData[2] | (((uint16_t)RxData[3])<<8);
						for(i=0;i<14;i++) {
							pos = 32*(node_addr&0x0F) + i*2+1;
							tmp = answer_90[pos]; // temporary buffer
							if(!(word_value & (1<<i))) {	// not fitted
								tmp&=0xFE;
							}else tmp|=0x01;
							if(pos<sizeof(answer_90)) answer_90[pos] = tmp;
						}
					}else if(intern_addr==5) {	// digital inputs
						word_value =RxData[2] | (((uint16_t)RxData[3])<<8);
						for(i=0;i<14;i++) {
							if(!(word_value & (1<<i))) {	// not fitted
								pos = 4*(node_addr&0x0F) + (i/4);
								tmp = answer_91[pos]; // temporary buffer
								byte_value = 0x03;
								byte_value = byte_value << (i%4)*2;
								tmp=tmp & (~byte_value); // clear value position
								byte_value = 0x02;	// not fitted code
								byte_value = byte_value << (i%4)*2;	// offset
								tmp|=byte_value;
								if(pos<sizeof(answer_91)) answer_91[pos] = tmp;
							}
						}
					}else if(intern_addr==6) {	// switch inputs
						word_value =RxData[2] | (((uint16_t)RxData[3])<<8);
						for(i=0;i<8;i++) {
							if(!(word_value & (1<<i))) {	// not fitted
								pos = 2*(node_addr&0x0F) + (i/4);
								tmp = answer_92[pos]; // temporary buffer
								byte_value = 0x03;
								byte_value = byte_value << (i%4)*2;
								tmp=tmp & (~byte_value); // clear value position
								byte_value = 0x02;	// not fitted code
								byte_value = byte_value << (i%4)*2;	// offset
								tmp|=byte_value;
								if(pos<sizeof(answer_92)) answer_92[pos] = tmp;
							}
						}
					}else if(intern_addr==7) {	// relay outputs
						word_value =RxData[2] | (((uint16_t)RxData[3])<<8);
						for(i=0;i<8;i++) {
							if(!(word_value & (1<<i))) {	// not fitted
								pos = 2*(node_addr&0x0F) + (i/4);
								tmp = answer_93[pos]; // temporary buffer
								byte_value = 0x03;
								byte_value = byte_value << (i%4)*2;
								tmp=tmp & (~byte_value); // clear value position
								byte_value = 0x02;	// not fitted code
								byte_value = byte_value << (i%4)*2;	// offset
								tmp|=byte_value;
								if(pos<sizeof(answer_93)) answer_93[pos] = tmp;
							}
						}
					}
					break;
				}
			}
		}
		if(clusters_update) {
			clusters_tmr++;
			if(clusters_tmr>=2000) {
				clusters_tmr = 0;
				clusters_update = 0;
				answer_9b[1] = clusters_state;
				for(i=0;i<8;i++) {
					if((answer_9b[1] & (1<<i))==0) {
						if(i<4) {
							for(tmp=0;tmp<32;tmp++)	answer_97[i*32+tmp]=0;
						}else {
							for(tmp=0;tmp<32;tmp++)	answer_98[(i-4)*32+tmp]=0;
						}
						answer_96[i*2] = 0;
						answer_96[i*2+1] = 0;
					}
				}
			}
		}
		osDelay(1);
	}
}
