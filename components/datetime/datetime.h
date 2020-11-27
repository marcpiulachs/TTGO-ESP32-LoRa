#ifndef DATETIME_H_
#define DATETIME_H_

#include <time.h>
#include <sys/time.h>

void datetime_init(void);
void datetime_reset_nvs(void);

#define TIME_READY_BIT BIT0

#endif