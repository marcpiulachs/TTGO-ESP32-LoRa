#pragma once

#include <stdint.h>


void Odroid_InitializeAudio(void);
void Odroid_PlayAudio(uint8_t* buffer, int length);