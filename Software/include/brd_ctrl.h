#ifndef __BRD_CTRL_H
#define __BRD_CTRL_H

#include <stdint.h>

void brd_ctrl_init(void);
bool brd_pwr_is_on(void);
void brd_pwr_on(void);
void brd_pwr_off(void);
void brd_pwr_toggle(void);

void brd_pwr_set(uint8_t state);

#endif