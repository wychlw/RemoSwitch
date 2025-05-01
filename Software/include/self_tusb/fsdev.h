#ifndef __SELF_TUSB_FSDEV_H__
#define __SELF_TUSB_FSDEV_H__

#include <stdint.h>
#include <stdbool.h>

void fsdev_int_handler(uint8_t rhport);
void fsdev_int_enable(uint8_t rhport);
void fsdev_int_disable(uint8_t rhport);
void fsdev_connect(uint8_t rhport);
void fsdev_disconnect(uint8_t rhport);
bool fsdev_init(uint8_t rhport, const tusb_rhport_init_t *rh_init);
void fsdev_sof_enable(uint8_t rhport, bool en);
void fsdev_set_address(uint8_t rhport, uint8_t dev_addr);
void fsdev_remote_wakeup(uint8_t rhport);
void fsdev_edpt0_status_complete(uint8_t rhport, tusb_control_request_t const *request);
bool fsdev_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const *desc_ep);
void fsdev_edpt_close_all(uint8_t rhport);
bool fsdev_edpt_iso_alloc(uint8_t rhport, uint8_t ep_addr, uint16_t largest_packet_size);
bool fsdev_edpt_iso_activate(uint8_t rhport, tusb_desc_endpoint_t const *desc_ep);
bool fsdev_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes);
bool fsdev_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t *ff, uint16_t total_bytes);
void fsdev_edpt_stall(uint8_t rhport, uint8_t ep_addr);
void fsdev_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr);


#endif