#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "ch32v20x.h"

#include "led.h"
#include "sd_mux.h"
#include "util.h"

static bool sd_mux_state = SD_MUX_HOST;
static bool sd_mux_open = true;

#define SET_LED_STAT()                                           \
    do {                                                         \
        sd_led_set(sd_mux_open && (sd_mux_state == SD_MUX_HOST), \
                   sd_mux_open && (sd_mux_state == SD_MUX_DUT)); \
    } while (0)

void sd_mux_init(void) {
    GPIO_InitTypeDef mux_en_init = {
        .GPIO_Pin = MUX_EN_PIN,
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Speed = GPIO_Speed_10MHz,
    };
    GPIO_Init(MUX_EN_PORT, &mux_en_init);

    GPIO_InitTypeDef sel0_init = {
        .GPIO_Pin = SEL0_PIN,
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Speed = GPIO_Speed_10MHz,
    };
    GPIO_Init(SEL0_PORT, &sel0_init);
    GPIO_InitTypeDef sel1_init = {
        .GPIO_Pin = SEL1_PIN,
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Speed = GPIO_Speed_10MHz,
    };
    GPIO_Init(SEL1_PORT, &sel1_init);
    GPIO_InitTypeDef tf_vcc_sel_init = {
        .GPIO_Pin = TF_VCC_SEL_PIN,
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Speed = GPIO_Speed_10MHz,
    };
    GPIO_Init(TF_VCC_SEL_PORT, &tf_vcc_sel_init);
    GPIO_InitTypeDef up_cho_init = {
        .GPIO_Pin = UP_CHO_PIN,
        .GPIO_Mode = GPIO_Mode_Out_PP,
        .GPIO_Speed = GPIO_Speed_10MHz,
    };
    GPIO_Init(UP_CHO_PORT, &up_cho_init);

    GPIO_WriteBit(MUX_EN_PORT, MUX_EN_PIN, 1);
    GPIO_WriteBit(SEL0_PORT, SEL0_PIN, 1);
    GPIO_WriteBit(SEL1_PORT, SEL1_PIN, sd_mux_state & 0x1);
    GPIO_WriteBit(TF_VCC_SEL_PORT, TF_VCC_SEL_PIN, sd_mux_state & 0x1);
    GPIO_WriteBit(UP_CHO_PORT, UP_CHO_PIN, sd_mux_state == SD_MUX_HOST ? 0 : 1);

    SET_LED_STAT();
}

void sd_mux_on(void) {
    GPIO_WriteBit(MUX_EN_PORT, MUX_EN_PIN, 1);
    SET_LED_STAT();
    delay_us(10);
}

void sd_mux_off(void) {
    GPIO_WriteBit(MUX_EN_PORT, MUX_EN_PIN, 0);
    SET_LED_STAT();
    delay_us(10);
}

bool sd_mux_is_on(void) {
    return sd_mux_open;
}

bool sd_mux_status(void) {
    return sd_mux_state;
}

void sd_mux_set(uint8_t host_or_dut) {
    if (sd_mux_state == host_or_dut) {
        return;
    }
    sd_mux_state = host_or_dut;

    if (sd_mux_state == SD_MUX_DUT) {
        GPIO_WriteBit(UP_CHO_PORT, UP_CHO_PIN, 1);
        delay_us(5);
    }

    GPIO_WriteBit(SEL1_PORT, SEL1_PIN, sd_mux_state & 0x1);
    GPIO_WriteBit(TF_VCC_SEL_PORT, TF_VCC_SEL_PIN, sd_mux_state & 0x1);

    if (sd_mux_state == SD_MUX_HOST) {
        GPIO_WriteBit(UP_CHO_PORT, UP_CHO_PIN, 0);
        delay_us(5);
    }
    SET_LED_STAT();
}

void sd_mux_toggle(void) {
    sd_mux_set(!sd_mux_state);
}