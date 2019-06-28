/*
 * canLogger.c
 *
 *  Created on: 13 янв. 2019 г.
 *      Author: User
 */

#include "canLogger.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "fsmc_flash.h"
#include "main.h"


struct logBuf logs[LOG_CNT];
extern RTC_HandleTypeDef hrtc;
unsigned short bl_num = 0;
unsigned short page_num = 0;

unsigned short busy_cnt = 0;

void initLogger() {
	int i = 0;
	for(i=0;i<LOG_CNT-1;i++) {
		logs[i].next = &logs[i+1];
		logs[i].fillFlag = 0;
	}
	logs[i].next = &logs[0];
	logs[i].fillFlag = 0;
}

struct logBuf* getFirstLog() {
	return &logs[0];
}

void canLoggerTask(void const * argument) {
	int i=0,j=0;
	static unsigned char res = 0;
	static unsigned char tst[2048];
	// test scan

	scan_bad_blocks();

	res=1;
	for(i=page_num;i<64;i++) {
		while(isFlashBusy()) {busy_cnt++;osDelay(1);}busy_cnt = 0;
		res = read_page(bl_num,i,tst);
		for(j=0;j<2048;j++) {if(tst[j]!=0xFF) res=0;break;}
		if(res==0) break;
	}
	if(res==0) {
		while(isFlashBusy()) {busy_cnt++;osDelay(1);}busy_cnt = 0;
		erase_block(bl_num);
	}

	for(;;)
	{
		for(i=0;i<LOG_CNT;i++) {
			if(logs[i].fillFlag) {	// необходимо записать страницу
				// поиск ближайшего неиспорченного блока
				while(check_block(bl_num)==0) {bl_num++;page_num=0;if(bl_num>=1024) bl_num=0;}
				while(isFlashBusy()) {busy_cnt++;osDelay(1);}

				if(page_num==0) {
					// поиск ближайшего неиспорченного блока
					while(check_block(bl_num)==0) {bl_num++;if(bl_num>=1024) bl_num=0;}
					while(isFlashBusy()) {busy_cnt++;osDelay(1);}
					busy_cnt = 0;
					res = erase_block(bl_num);
					//HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
					// ошибка при стирании
					while(res==FLASH_ERROR) {
						add_bad_block(bl_num);
						while(isFlashBusy()) {busy_cnt++;osDelay(1);}
						busy_cnt = 0;
						//mark_bad_block(bl_num);
						while(check_block(bl_num)==0) {bl_num++;page_num=0;if(bl_num>=1024) bl_num=0;}
						while(isFlashBusy()) {busy_cnt++;osDelay(1);}
						busy_cnt = 0;
						res = erase_block(bl_num);
						osDelay(1);
					}
				}

				//vTaskSuspendAll();

				while(isFlashBusy()) {busy_cnt++;osDelay(1);}busy_cnt = 0;
				for(j=0;j<2048;j++) tst[j] = 0xFF;
				res = read_page(bl_num,page_num,tst);
				//res = 1;

				//for(j=0;j<2048;j++) if(tst[j]!=0xFF) {res=0;break;}
				//if(res) HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
				while(isFlashBusy()) {busy_cnt++;osDelay(1);}busy_cnt = 0;
				res = write_page(bl_num,page_num,logs[i].buf);
				for(j=0;j<2048;j++) tst[j] = 0;
				res = read_page(bl_num,page_num,tst);

#ifdef DEBUG_MODE
				if(res==FLASH_OK) {
					res = 1;
					for(j=0;j<10;j++) if(tst[j]!=logs[i].buf[j]) {res=0;break;}
					if(res) HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
				}

				//xTaskResumeAll();
#endif
				// ошибка при записи
				while(res==FLASH_ERROR) {

					add_bad_block(bl_num);
					while(isFlashBusy()) {busy_cnt++;osDelay(1);}
					busy_cnt = 0;
					//mark_bad_block(bl_num);
					while(check_block(bl_num)==0) {bl_num++;page_num=0;if(bl_num>=1024) bl_num=0;}
					while(isFlashBusy()) {busy_cnt++;osDelay(1);}
					busy_cnt = 0;
					res = write_page(bl_num,page_num,logs[i].buf);
				}
				//if(res==FLASH_OK) HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
				clearLogBuf(&logs[i]);
				// увеличение номера страницы
				page_num++;if(page_num>=64) {page_num=0;bl_num++;if(bl_num>=1024) bl_num=0;}
				HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, bl_num);
				HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, page_num);
				logs[i].fillFlag = 0;
				osDelay(1);
			}
		}
		osDelay(1);
	}
}

void clearLogBuf(struct logBuf *ptr) {
	int i=0;
	for(i=0;i<2048;i++) ptr->buf[i] = 0;
}
