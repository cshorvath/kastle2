#pragma once
#include <stdint.h>
typedef int (*SamSink)(void *context, uint8_t sample);
typedef uint16_t (*SamPitch)(void *context);
/* Single-owner engine. A sink returning zero cancels rendering immediately. */
int SamSpeak(const char *text, uint8_t speed, uint8_t mouth, uint8_t throat,
             int sing, SamSink sink, SamPitch pitch, void *context);
void SamWrite(int position, uint8_t value);
uint8_t SamPeriod(uint8_t period);
