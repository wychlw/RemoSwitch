#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board_api.h"
#include "ch32v20x.h"
#include "tusb.h"

#include "cdc.h"
#include "led.h"
#include "queued_task.h"
#include "sd_mux.h"
#include "usb.h"
#include "util.h"
#include "write.h"

static void init() {
    __disable_irq();

    uint8_t hse_clk = clock_init();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    queued_task_init();
    print_init();
    led_init();
    sd_mux_init();
    usb_init();

    __enable_irq();


    if (hse_clk == 0) {
        error("HSE clock error.\r\n");
        warn("Using HSI clock.\r\n");
    } else {
        info("HSE clock ok.\r\n");
        info("HSE Clk:\t%d MHz\r\n", hse_clk);
    }
    info("Core Clk:\t%d Hz\r\n", SystemCoreClock);


    info("Sys Clk:\t\t%d Hz\r\n", clocks.SYSCLK_Frequency);
    info("AHB/HCLK Clk:\t\t%d Hz\r\n", clocks.HCLK_Frequency);
    info("APB1/PCLK1 Clk:\t\t%d Hz\r\n", clocks.PCLK1_Frequency);
    info("APB2/PCLK2 Clk:\t\t%d Hz\r\n", clocks.PCLK2_Frequency);
    info("ADC Clk:\t\t%d Hz\r\n", clocks.ADCCLK_Frequency);

    info("System init done.\r\n");

    
    watchdog_init();
}

static bool can_continue = false;
static void can_continue_set() {
    can_continue = true;
}
int main(void) {
    init();

    info("Device init finish.\r\n");
    info("ChipID:\t0x%08x\r\n", DBGMCU_GetCHIPID());

    // wait a while for everything to stabilize
    queued_task_add(can_continue_set, 50, false);
    while (!can_continue) {
        watchdog_task();
    }

    while (1) {
        tud_task();
        cdc_task();
        led_task();
        watchdog_task();
    }
}
