#ifndef DATETIME_H_
#define DATETIME_H_

#include <time.h>
#include <sys/time.h>

void dateTimeInit(void);
void datTimeResetNVS(void);

#define TIME_READY_BIT BIT0

#endif