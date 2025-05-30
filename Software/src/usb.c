#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "bsp/board_api.h"
#include "ch32v20x.h"
#include "tusb.h"

#define TUP_USBIP_CONTROLLER_NUM 4

#include "usb.h"

#include "cdc.h"
#include "queued_task.h"
#include "sd_mux.h"
#include "write.h"

/*
 * Switch buf[1]:
 *      0: MUX_EN = buf[2]
 *      1: SEL0 = buf[2]
 *      2: SEL1 = buf[2]
 *      3: TF_VCC_SEL = buf[2]
 *      4: UP_CHO = buf[2]
 *      5: SD_HOST_LED = buf[2]
 *      6: SD_TS_LED = buf[2]
 *      7: UART send buf[2]
*/
static void sd_mux_raw_pin_handle(uint8_t const *buf, uint16_t len) {
    if (len < 3) {
        error("SD_MUX raw pin report too short\r\n");
        return;
    }
    switch (buf[1]) {
        case 0:
            GPIO_WriteBit(MUX_EN_PORT, MUX_EN_PIN, buf[2] ? 1 : 0);
            info("MUX_EN set to %d\r\n", buf[2]);
            break;
        case 1:
            GPIO_WriteBit(SEL0_PORT, SEL0_PIN, buf[2] ? 1 : 0);
            info("SEL0 set to %d\r\n", buf[2]);
            break;
        case 2:
            GPIO_WriteBit(SEL1_PORT, SEL1_PIN, buf[2] ? 1 : 0);
            info("SEL1 set to %d\r\n", buf[2]);
            break;
        case 3:
            GPIO_WriteBit(TF_VCC_SEL_PORT, TF_VCC_SEL_PIN, buf[2] ? 1 : 0);
            info("TF_VCC_SEL set to %d\r\n", buf[2]);
            break;
        case 4:
            GPIO_WriteBit(UP_CHO_PORT, UP_CHO_PIN, buf[2] ? 1 : 0);
            info("UP_CHO set to %d\r\n", buf[2]);
            break;
        case 5:
            GPIO_WriteBit(SD_HOST_LED_PORT, SD_HOST_LED_PIN, buf[2] ? 1 : 0);
            info("SD_HOST_LED set to %d\r\n", buf[2]);
            break;
        case 6:
            GPIO_WriteBit(SD_TS_LED_PORT, SD_TS_LED_PIN, buf[2] ? 1 : 0);
            info("SD_TS_LED set to %d\r\n", buf[2]);
            break;
        case 7:
            print("UART send: %02x\r\n", buf[2]);
            break;
    }
}

/*
 * switch buf[1]:
 *      0: close mux
 *      1: open mux
 *      2: set to host
 *      3: set to dut
 *      4: query status, 
 *          [0, 4, mux is on/off, to_host, to_dut]
*/
static void sd_mux_hid_handle(uint8_t const *buf, uint16_t len) {
    if (len < 2) {
        error("SD_MUX control HID report too short\r\n");
        return;
    }
    static uint8_t report_buffer[] = {0, 4, 0, 0, 0};
    switch (buf[1]) {
        case 0:
            sd_mux_off();
            info("MUX is now OFF!\r\n");
            break;
        case 1:
            sd_mux_on();
            info("MUX is now ON!\r\n");
            break;
        case 2:
            sd_mux_set(SD_MUX_HOST);
            info("MUX set to DUT\r\n");
            break;
        case 3:
            sd_mux_set(SD_MUX_DUT);
            info("MUX set to HOST\r\n");
            break;
        case 4:
            report_buffer[0] = 0;
            report_buffer[1] = 4;
            report_buffer[2] = sd_mux_is_on();
            report_buffer[3] = sd_mux_status() == SD_MUX_HOST ? 1 : 0;
            report_buffer[4] = sd_mux_status() == SD_MUX_DUT ? 1 : 0;
            tud_hid_report(0, report_buffer, sizeof(report_buffer));
            break;
        default:
            error("Unknown sd_mux control command: %02x\r\n", buf[1]);
    }
}

/**
 * buf[0] = 0: Control SD MUX
 * buf[0] = 0xFF: Echo Back
 */
static void hid_handle(uint8_t const *buf, uint16_t len) {
    if (len < 1) {
        error("HID report too short\r\n");
        return;
    }
    if (buf[0] == 0) {
        sd_mux_hid_handle(buf, len);
        return;
    }
    if (buf[0] == 1) {
        sd_mux_raw_pin_handle(buf, len);
        return;
    }
    if (buf[0] == 0xFF) {
        tud_hid_report(0, buf, len);
        info("HID echo back: ");
        for (uint16_t i = 0; i < len; i++) {
            print("0x%02x ", buf[i]);
        }
        print("\r\n");
        return;
    }
    error("Unknown hid command: %02x\r\n", buf[0]);
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;

    if (reqlen > 0) {
        buffer[0] = 0;
        return 1;
    }
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    (void) itf;
    (void) report_id;
    (void) report_type;

    if (bufsize < 1) {
        return;
    }
    hid_handle(buffer, bufsize);
}

extern tusb_role_t _tusb_rhport_role[TUP_USBIP_CONTROLLER_NUM];
bool custom_tusb_init(uint8_t rhport, const tusb_rhport_init_t *rh_init) {
    TU_ASSERT(rhport < TUP_USBIP_CONTROLLER_NUM && rh_init->role != TUSB_ROLE_INVALID);
    _tusb_rhport_role[rhport] = rh_init->role;

#if CFG_TUD_ENABLED
    if (rh_init->role == TUSB_ROLE_DEVICE) {
        return tud_rhport_init(rhport, rh_init);
    }
#endif

#if CFG_TUH_ENABLED
    if (rh_init->role == TUSB_ROLE_HOST) {
        return tud_rhport_init(rhport, rh_init);
    }
#endif

    return true;
}

void usb_init(void) {
    uint8_t usb_div;
    switch (SystemCoreClock) {
        case 48000000:
            usb_div = RCC_USBCLKSource_PLLCLK_Div1;
            break;
        case 96000000:
            usb_div = RCC_USBCLKSource_PLLCLK_Div2;
            break;
        case 144000000:
            usb_div = RCC_USBCLKSource_PLLCLK_Div3;
            break;
        default:
            TU_ASSERT(0, );
            break;
    }
    RCC_USBCLKConfig(usb_div);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE); // FSDEV
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);// USB FS


#ifdef FSDEV_TUD_RHPORT
    tusb_rhport_init_t dev_init1 = {.role = TUSB_ROLE_DEVICE,
                                    .speed = TUSB_SPEED_AUTO};
    if (!custom_tusb_init(FSDEV_TUD_RHPORT, &dev_init1)) {
        error("USB FSDEV init failed\r\n");
    }
#endif
#ifdef USBFS_TUD_RHPORT
    tusb_rhport_init_t dev_init2 = {.role = TUSB_ROLE_DEVICE,
                                    .speed = TUSB_SPEED_AUTO};
    if (!custom_tusb_init(USBFS_TUD_RHPORT, &dev_init2)) {
        error("USB FSDEV init failed\r\n");
    }
#endif
}
