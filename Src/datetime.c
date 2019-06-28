/*
 * datetime.c
 *
 *  Created on: 8 авг. 2018 г.
 *      Author: Roman
 */

#include "datetime.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#define JD0 2451545 // дней до 01 янв 2000 ПН

extern RTC_HandleTypeDef hrtc;
static RTC_TimeTypeDef sTime;
static RTC_DateTypeDef sDate;
static ftime_t sdt1;
static sDateTime sdt2;

unsigned long cTime = 0;

const uint16_t day_offset[12] = {0, 31, 61,92, 122, 153, 184, 214, 245, 275,306, 337};

static unsigned long EncodeDateTime(sDateTime *dt)
{
	uint8_t a = dt->month < 3; // а = 1, если мес¤ц ¤нварь или февраль
	if((dt->year==2000)&&(a)) {
		if(dt->month==1) return 86400*(dt->date-1) + (int)dt->hour * 3600 + (int)dt->min * 60 + dt->sec;
		else return 86400*(dt->date-1) + (unsigned long)31*86400 + (int)dt->hour * 3600 + (int)dt->min * 60 + dt->sec;
	}else {
		int16_t y = dt->year - a - 2000;  // y = отнимаем от года 1, если а = 1, а так же 2000;
		uint8_t m = dt->month + 12 * a - 3; // аналоги¤ выражени¤ (12 + month - 3) % 12; делаем март нулевым мес¤цем года.
		return 5184000 + (dt->date - 1 + day_offset[m] + y * 365 + y / 4 - y / 100 + y / 400) * 86400 +
			   (int)dt->hour * 3600 + (int)dt->min * 60 + dt->sec;
	}
}

unsigned long counterFromTime(void) {
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	//portDISABLE_INTERRUPTS();
	sdt2.sec = sTime.Seconds;
	sdt2.min = sTime.Minutes;
	sdt2.hour = sTime.Hours;
	sdt2.date = sDate.Date;
	sdt2.month = sDate.Month;
	sdt2.year = sDate.Year + 2000;
	cTime = EncodeDateTime(&sdt2);
	return cTime;
}

void datetimeTask(void const * argument) {

	counterFromTime();

	for(;;)
	{
		counterFromTime();
		//portENABLE_INTERRUPTS();
		osDelay(1000);
	}
}

void updateCurrentTime(unsigned long counter) {

	uint32_t ace;
	uint8_t b;
	uint8_t d;
	uint8_t m;

	ace=(counter/86400)+32044+JD0;
	b=(4*ace+3)/146097;
	ace=ace-((146097*b)/4);
	d=(4*ace+3)/1461;
	ace=ace-((1461*d)/4);
	m=(5*ace+2)/153;

	portDISABLE_INTERRUPTS();

	sdt1.day=ace-((153*m+2)/5)+1;
	sdt1.month=m+3-(12*(m/10));
	sdt1.year=100*b+d-4800+(m/10);
	sdt1.hour=(counter/3600)%24;
	sdt1.minute=(counter/60)%60;
	sdt1.second=(counter%60);

	sTime.Seconds = sdt1.second;
	sTime.Minutes = sdt1.minute;
	sTime.Hours = sdt1.hour;
	sDate.Date = sdt1.day;
	sDate.Month = sdt1.month;
	if(sdt1.year>=2000) sDate.Year = sdt1.year - 2000;else sDate.Year = 0;
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	cTime = counter;

	portENABLE_INTERRUPTS();
}
