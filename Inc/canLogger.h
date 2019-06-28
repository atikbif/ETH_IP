/*
 * canLogger.h
 *
 *  Created on: 13 џэт. 2019 у.
 *      Author: User
 */

#ifndef CANLOGGER_H_
#define CANLOGGER_H_

#define LOG_CNT	2

struct logBuf {
	unsigned char buf[2048];
	unsigned char fillFlag;
	struct logBuf *next;
};

void initLogger();
struct logBuf* getFirstLog();
void canLoggerTask(void const * argument);
void clearLogBuf(struct logBuf *ptr);

#endif /* CANLOGGER_H_ */
