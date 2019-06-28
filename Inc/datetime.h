/*
 * datetime.h
 *
 *  Created on: 8 рту. 2018 у.
 *      Author: Roman
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include <stdlib.h>
#include "stm32f4xx_hal.h"

typedef struct{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} ftime_t;

typedef struct
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t date;
	uint8_t month;
	uint8_t day;
	int16_t year;
} sDateTime;

void updateCurrentTime(unsigned long counter);

void datetimeTask(void const * argument);
unsigned long counterFromTime(void);

#endif /* DATETIME_H_ */
