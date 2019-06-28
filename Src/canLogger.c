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
	int i=0;
	static unsigned char res = 0;

	// test scan

	//scan_bad_blocks();

	for(;;)
	{
		for(i=0;i<LOG_CNT;i++) {
			if(logs[i].fillFlag) {	// необходимо записать страницу
				// поиск ближайшего неиспорченного блока
				while(check_block(bl_num)==0) {bl_num++;page_num=0;if(bl_num>=1024) bl_num=0;}
				while(isFlashBusy()) {busy_cnt++;osDelay(1);}
				busy_cnt = 0;
				res = write_page(bl_num,page_num,logs[i].buf);

				//if(res==FLASH_OK && logs[i].buf[5]!=0xFF) HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);

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
				//HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
				clearLogBuf(&logs[i]);
				// увеличение номера страницы
				page_num++;if(page_num>=64) {page_num=0;bl_num++;if(bl_num>=1024) bl_num=0;}
				// проверка на необходимость стирания следующего блока
				if(page_num==0) {
					// поиск ближайшего неиспорченного блока
					while(check_block(bl_num)==0) {bl_num++;if(bl_num>=1024) bl_num=0;}
					while(isFlashBusy()) {busy_cnt++;osDelay(1);}
					busy_cnt = 0;
					res = erase_block(bl_num);
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
