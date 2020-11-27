#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>

typedef struct
{
	uint8_t* buffer;
	int length;
} audio_r;

void audio_init(void);
void audioPlay(uint8_t* buffer, int length);
void audioPlayAuthorized(void);
void audioPlayDenied(void);

#endif