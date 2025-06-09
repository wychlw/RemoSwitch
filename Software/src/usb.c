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
#include "util.h"
#include "write.h"

/*
 * Switch buf[1]:
 *      0: MUX_EN = buf[2]
 *      1: SEL0 = buf[2]
 *      2: SEL1 = buf[2]
 *      3: TF_VCC_EN = buf[2]
 *      4: DOWN_VCC_EN = buf[2]
 *      5: UP_VCC_EN = buf[2]
 *      6: UP_CHO = buf[2]
 *      7: SD_HOST_LED = buf[2]
 *      8: SD_TS_LED = buf[2]
 *      9: BRD_PWR_EN = buf[2]
 *      a: UART send buf[2]
 *      b: Query all GPIOs
*/
static void sd_mux_raw_pin_handle(uint8_t const *buf, uint16_t len) {
    if (len < 3) {
        error("SD_MUX raw pin report too short\r\n");
        error("Full command: ");
        for (uint16_t i = 0; i < len; i++) {
            print("0x%02x ", buf[i]);
        }
        print("\r\n");
        return;
    }
    switch (buf[1]) {
        case 0x00:
            GPIO_WriteBit(MUX_EN_PORT, MUX_EN_PIN, buf[2] ? 1 : 0);
            info("MUX_EN set to %d\r\n", buf[2]);
            break;
        case 0x01:
            GPIO_WriteBit(SEL0_PORT, SEL0_PIN, buf[2] ? 1 : 0);
            info("SEL0 set to %d\r\n", buf[2]);
            break;
        case 0x02:
            GPIO_WriteBit(SEL1_PORT, SEL1_PIN, buf[2] ? 1 : 0);
            info("SEL1 set to %d\r\n", buf[2]);
            break;
        case 0x03:
            GPIO_WriteBit(TF_VCC_EN_PORT, TF_VCC_EN_PIN, buf[2] ? 1 : 0);
            info("TF_VCC_EN_PORT set to %d\r\n", buf[2]);
            break;
        case 0x04:
            GPIO_WriteBit(DOWN_VCC_EN_PORT, DOWN_VCC_EN_PIN, buf[2] ? 1 : 0);
            info("DOWN_VCC_EN_PORT set to %d\r\n", buf[2]);
            break;
        case 0x05:
            GPIO_WriteBit(UP_VCC_EN_PORT, UP_VCC_EN_PIN, buf[2] ? 1 : 0);
            info("UP_VCC_EN_PORT set to %d\r\n", buf[2]);
            break;
        case 0x06:
            GPIO_WriteBit(UP_CHO_PORT, UP_CHO_PIN, buf[2] ? 1 : 0);
            info("UP_CHO set to %d\r\n", buf[2]);
            break;
        case 0x07:
            GPIO_WriteBit(SD_HOST_LED_PORT, SD_HOST_LED_PIN, buf[2] ? 1 : 0);
            info("SD_HOST_LED set to %d\r\n", buf[2]);
            break;
        case 0x08:
            GPIO_WriteBit(SD_TS_LED_PORT, SD_TS_LED_PIN, buf[2] ? 1 : 0);
            info("SD_TS_LED set to %d\r\n", buf[2]);
            break;
        case 0x09:
            GPIO_WriteBit(BRD_PWR_EN_PORT, BRD_PWR_EN_PIN, buf[2] ? 1 : 0);
            info("BRD_PWR_EN set to %d\r\n", buf[2]);
            break;
        case 0x0a:
            print("UART send: %02x\r\n", buf[2]);
            break;
        case 0x0b:
            print("Here reports all GPIOs:\r\n");
            print("MUX_EN: %d\r\n", GPIO_ReadInputDataBit(MUX_EN_PORT, MUX_EN_PIN));
            print("SEL0: %d\r\n", GPIO_ReadInputDataBit(SEL0_PORT, SEL0_PIN));
            print("SEL1: %d\r\n", GPIO_ReadInputDataBit(SEL1_PORT, SEL1_PIN));
            print("TF_VCC_EN: %d\r\n", GPIO_ReadInputDataBit(TF_VCC_EN_PORT, TF_VCC_EN_PIN));
            print("DOWN_VCC_EN: %d\r\n", GPIO_ReadInputDataBit(DOWN_VCC_EN_PORT, DOWN_VCC_EN_PIN));
            print("UP_VCC_EN: %d\r\n", GPIO_ReadInputDataBit(UP_VCC_EN_PORT, UP_VCC_EN_PIN));
            print("UP_CHO: %d\r\n", GPIO_ReadInputDataBit(UP_CHO_PORT, UP_CHO_PIN));
            print("SD_HOST_LED: %d\r\n", GPIO_ReadInputDataBit(SD_HOST_LED_PORT, SD_HOST_LED_PIN));
            print("SD_TS_LED: %d\r\n", GPIO_ReadInputDataBit(SD_TS_LED_PORT, SD_TS_LED_PIN));
            print("SD_MUX is %s\r\n", sd_mux_is_on() ? "ON" : "OFF");
            print("SD_MUX status is %s\r\n", sd_mux_status() == SD_MUX_HOST ? "HOST" : "DUT");
            print("SD_MUX open is %s\r\n", sd_mux_is_on() ? "OPEN" : "CLOSED");
            print("BRD_PWR_EN is %s\r\n", GPIO_ReadInputDataBit(BRD_PWR_EN_PORT, BRD_PWR_EN_PIN) ? "ON" : "OFF");
            break;
        default:
            error("Unknown debug control command: %02x\r\n", buf[1]);
            error("Full command: ");
            for (uint16_t i = 0; i < len; i++) {
                print("0x%02x ", buf[i]);
            }
            print("\r\n");
    }
}

/*
 * switch buf[1]:
 *      0: close mux
 *      1: open mux
 *      2: set to host
 *      3: set to dut
 *      4: query status,
 *          [mux is on/off, to_host, to_dut, brd power on/off]
 *      5: device id
 *      6: set brd pwr on
 *      7: set brd pwr off
*/
static void sd_mux_hid_handle(uint8_t const *buf, uint16_t len) {
    if (len < 2) {
        error("SD_MUX control HID report too short\r\n");
        return;
    }
    info("SD_MUX control command: %02x\r\n", buf[1]);
    static uint8_t report_buffer[16] = {0};
    switch (buf[1]) {
        case 0x0:
            sd_mux_off();
            info("MUX is now OFF!\r\n");
            break;
        case 0x1:
            sd_mux_on();
            info("MUX is now ON!\r\n");
            break;
        case 0x2:
            sd_mux_set(SD_MUX_HOST);
            info("MUX set to HOST\r\n");
            break;
        case 0x3:
            sd_mux_set(SD_MUX_DUT);
            info("MUX set to DUT\r\n");
            break;
        case 0x4:
            report_buffer[0] = sd_mux_is_on();
            report_buffer[1] = sd_mux_status() == SD_MUX_HOST ? 1 : 0;
            report_buffer[2] = sd_mux_status() == SD_MUX_DUT ? 1 : 0;
            report_buffer[3] = brd_pwr_is_on() ? 1 : 0;
            tud_hid_report(0, report_buffer, sizeof(uint8_t[4]));
            break;
        case 0x5:;
            uint32_t id = device_id();
            report_buffer[0] = (id >> 24) & 0xFF;
            report_buffer[1] = (id >> 16) & 0xFF;
            report_buffer[2] = (id >> 8) & 0xFF;
            report_buffer[3] = (id >> 0) & 0xFF;
            tud_hid_report(0, report_buffer, sizeof(uint8_t[4]));
            break;
        case 0x6:
            brd_pwr_on();
            info("Board power is ON\r\n");
            break;
        case 0x7:
            brd_pwr_off();
            info("Board power is OFF\r\n");
            break;
        default:
            error("Unknown sd_mux control command: %02x\r\n", buf[1]);
            error("Full command: ");
            for (uint16_t i = 0; i < len; i++) {
                print("0x%02x ", buf[i]);
            }
            print("\r\n");
    }
}

/**
 * buf[0] = 0: Control SD MUX
 * buf[0] = 1: Raw Pin Control
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
    error("Full command: ");
    for (uint16_t i = 0; i < len; i++) {
        print("0x%02x ", buf[i]);
    }
    print("\r\n");
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
        error("USB USBFS init failed\r\n");
    }
#endif
}
