/*
 * canViewer.c
 *
 *  Created on: 22 апр. 2019 г.
 *      Author: User
 */


#include "canViewer.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#define VIEW_TIME	3000
#define UPD_TIME	10000

extern CAN_HandleTypeDef hcan1;
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

static uint16_t node_tmr[7] = {0,0,0,0,0,0,0};

static uint16_t clusters_tmr = 0;
static uint8_t clusters_update = 0;
static uint8_t clusters_state = 0;

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
	if((*tmr)==VIEW_TIME) {
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
	}else if((*tmr)==VIEW_TIME+50) {
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
	(*tmr)++;
	if((*tmr)==UPD_TIME) {	// device type
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
	}else if((*tmr)==UPD_TIME+50) {	// id
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
	}else if((*tmr)==UPD_TIME+100) {	// telemetry tx mask (node-0, analog)
		TxHeader.StdId = 0x06;
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
	}else if((*tmr)==UPD_TIME+150) {	// telemetry tx mask (node-0, digital inp)
		TxHeader.StdId = 0x06;
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
	}else if((*tmr)==UPD_TIME+200) {	// telemetry tx mask (node-1, switch input)
		TxHeader.StdId = 0x0E;
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
	}/*else if((*tmr)==UPD_TIME+250) {	// telemetry tx mask (node-3, analog)
		TxHeader.StdId = 0x06 | (0x03<<3);
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
	}*/else if((*tmr)>UPD_TIME+250) {(*tmr)=0;}
}

void canViewerTask(void const * argument) {
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

	for(;;)
	{
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
				//answer_91[0]++;
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
								//if(pos<sizeof(answer_91)) answer_91[pos] = tmp;
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
								if(RxData[4]) answer_90[pos] = 0xFD;
							}else if(RxData[3]==0x04)
							{
								//pos--;
								//answer_90[pos++] = RxData[7];
								//answer_90[pos] = RxData[6];
								//word_value = word_value/2;
								if(word_value<=180*2) {
									answer_90[pos] = 0xFD;
								}else if(word_value<=180*3.5) { // 350
									answer_90[pos] = 0xEF;
								}else if(word_value>=180*22) {
									answer_90[pos] = 0xFD;
								}else if(word_value>=180*20.5) {
									answer_90[pos] = 0xF7;
								}else answer_90[pos] = 0xFF;
								//if(RxData[4]) answer_90[pos] = 0xFD;
							}else {
								if(RxData[4]) answer_90[pos] = 0xFD;
								else answer_90[pos] = 0xFF;
							}
							//tmp = 0x60|0x02;
							//if(RxData[4]) answer_90[pos]&=~(tmp);else answer_90[pos]|=tmp;
							//answer_90[pos]|=0x01;
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
					/*answer_9b[1] = RxData[2];
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
					}*/
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
				/*if(((RxData[0] & 0x1F)==0x1F)&&(RxData[1]==0x0E)&&(RxHeader.DLC==6)) {
					extTime = RxData[5];
					extTime<<=8;extTime|=RxData[4];
					extTime<<=8;extTime|=RxData[3];
					extTime<<=8;extTime|=RxData[2];
					updateCurrentTime(extTime);
				}*/
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
