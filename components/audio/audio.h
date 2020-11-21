#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>

void audioInit(void);
void audioPlay(uint8_t* buffer, int length);

#endif