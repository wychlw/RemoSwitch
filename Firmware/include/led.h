#ifndef __LED_H
#define __LED_H

#include <stdbool.h>

#define BLINK_INTERVAL 1000

void led_init(void);
void led_task(void);

void sd_led_set(bool host, bool ts);

#endif