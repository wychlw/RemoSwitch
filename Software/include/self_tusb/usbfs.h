#ifndef __SELF_TUSB_USBFS_H__
#define __SELF_TUSB_USBFS_H__

#include <stdbool.h>
#include <stdint.h>

bool usbfs_init(uint8_t rhport, const tusb_rhport_init_t *rh_init);
void usbfs_int_handler(uint8_t rhport);
void usbfs_int_enable(uint8_t rhport);
void usbfs_int_disable(uint8_t rhport);
void usbfs_connect(uint8_t rhport);
void usbfs_disconnect(uint8_t rhport);
void usbfs_set_address(uint8_t rhport, uint8_t dev_addr);
void usbfs_remote_wakeup(uint8_t rhport);
void usbfs_sof_enable(uint8_t rhport, bool en);
void usbfs_edpt0_status_complete(uint8_t rhport, tusb_control_request_t const *request);
bool usbfs_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const *desc_ep);
void usbfs_edpt_close_all(uint8_t rhport);
void usbfs_edpt_close(uint8_t rhport, uint8_t ep_addr);
bool usbfs_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes);
void usbfs_edpt_stall(uint8_t rhport, uint8_t ep_addr);
void usbfs_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr);

#endif