#ifndef __LED_H
#define __LED_H
#include "sys.h"
#define LED0 PAout(4)
#define LED1 PCout(13)
void LED_Init(void);
#endif
