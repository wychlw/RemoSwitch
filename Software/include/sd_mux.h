#ifndef __SD_MUX_H
#define __SD_MUX_H

#include <stdint.h>

#define SD_MUX_HOST 1
#define SD_MUX_DUT  0

void sd_mux_init(void);
bool sd_mux_is_on(void);
bool sd_mux_status(void);
void sd_mux_on(void);
void sd_mux_off(void);
void sd_mux_set(uint8_t host_or_dut);
void sd_mux_toggle(void);

#endif