#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "ch32v20x.h"

#include "brd_ctrl.h"

static bool brd_ctrl_state = false;

void brd_ctrl_init(void) {
    GPIO_InitTypeDef brd_ctrl_init = {
        .GPIO_Pin = BRD_PWR_EN_PIN,
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Speed = GPIO_Speed_10MHz,
    };
    GPIO_Init(BRD_PWR_EN_PORT, &brd_ctrl_init);
    brd_ctrl_off();
}

void brd_pwr_on(void) {
    GPIO_WriteBit(BRD_PWR_EN_PORT, BRD_PWR_EN_PIN, 0);
    brd_ctrl_state = true;
}

void brd_pwr_off(void) {
    GPIO_WriteBit(BRD_PWR_EN_PORT, BRD_PWR_EN_PIN, 1);
    brd_ctrl_state = false;
}

bool brd_pwr_is_on(void) {
    return brd_ctrl_state;
}

void brd_pwr_toggle(void) {
    if (brd_ctrl_state) {
        brd_ctrl_off();
    } else {
        brd_ctrl_on();
    }
}

void brd_pwr_set(uint8_t state) {
    if (state) {
        brd_ctrl_on();
    } else {
        brd_ctrl_off();
    }
}