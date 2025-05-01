#include "tusb.h"
#include "self_tusb/fsdev.h"
#include "self_tusb/usbfs.h"
#include "tusb_config.h"
#include <stdbool.h>

#if defined(CFG_TUD_WCH_USBIP_MERGE) && CFG_TUD_WCH_USBIP_MERGE \
    && !(CFG_TUD_WCH_USBIP_FSDEV || CFG_TUD_WCH_USBIP_USBFS)

#ifndef FSDEV_TUD_RHPORT
#define FSDEV_TUD_RHPORT 1000
#endif

#ifndef USBFS_TUD_RHPORT
#define USBFS_TUD_RHPORT 1001
#endif


bool dcd_init(uint8_t rhport, const tusb_rhport_init_t *rh_init) {
    if (rhport == FSDEV_TUD_RHPORT) {
        return fsdev_init(rhport, rh_init);
    } else if (rhport == USBFS_TUD_RHPORT) {
        return usbfs_init(rhport, rh_init);
    }
    return false;
}

void dcd_int_handler(uint8_t rhport) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_int_handler(rhport);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_int_handler(rhport);
    }
}

void dcd_int_enable(uint8_t rhport) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_int_enable(rhport);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_int_enable(rhport);
    }
}


void dcd_int_disable(uint8_t rhport) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_int_disable(rhport);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_int_disable(rhport);
    }
}

void dcd_connect(uint8_t rhport) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_connect(rhport);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_connect(rhport);
    }
}

void dcd_disconnect(uint8_t rhport) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_disconnect(rhport);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_disconnect(rhport);
    }
}

void dcd_set_address(uint8_t rhport, uint8_t dev_addr) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_set_address(rhport, dev_addr);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_set_address(rhport, dev_addr);
    }
}

void dcd_sof_enable(uint8_t rhport, bool en) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_sof_enable(rhport, en);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_sof_enable(rhport, en);
    }
}

void dcd_remote_wakeup(uint8_t rhport) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_remote_wakeup(rhport);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_remote_wakeup(rhport);
    }
}

void dcd_edpt0_status_complete(uint8_t rhport, tusb_control_request_t const *request) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_edpt0_status_complete(rhport, request);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_edpt0_status_complete(rhport, request);
    }
}

bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const *desc_ep) {
    if (rhport == FSDEV_TUD_RHPORT) {
        return fsdev_edpt_open(rhport, desc_ep);
    } else if (rhport == USBFS_TUD_RHPORT) {
        return usbfs_edpt_open(rhport, desc_ep);
    }
    return false;
}

void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr) {
    if (rhport == FSDEV_TUD_RHPORT) {
        return;
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_edpt_close(rhport, ep_addr);
    }
}

void dcd_edpt_close_all(uint8_t rhport) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_edpt_close_all(rhport);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_edpt_close_all(rhport);
    }
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes) {
    if (rhport == FSDEV_TUD_RHPORT) {
        return fsdev_edpt_xfer(rhport, ep_addr, buffer, total_bytes);
    } else if (rhport == USBFS_TUD_RHPORT) {
        return usbfs_edpt_xfer(rhport, ep_addr, buffer, total_bytes);
    }
    return false;
}

void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_edpt_stall(rhport, ep_addr);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_edpt_stall(rhport, ep_addr);
    }
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr) {
    if (rhport == FSDEV_TUD_RHPORT) {
        fsdev_edpt_clear_stall(rhport, ep_addr);
    } else if (rhport == USBFS_TUD_RHPORT) {
        usbfs_edpt_clear_stall(rhport, ep_addr);
    }
}

bool dcd_edpt_iso_alloc(uint8_t rhport, uint8_t ep_addr, uint16_t largest_packet_size) {
    if (rhport == FSDEV_TUD_RHPORT) {
        return fsdev_edpt_iso_alloc(rhport, ep_addr, largest_packet_size);
    } 
    return false;
}

bool dcd_edpt_iso_activate(uint8_t rhport, tusb_desc_endpoint_t const *desc_ep) {
    if (rhport == FSDEV_TUD_RHPORT) {
        return fsdev_edpt_iso_activate(rhport, desc_ep);
    } 
    return false;
}

bool dcd_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t *ff, uint16_t total_bytes) {
    if (rhport == FSDEV_TUD_RHPORT) {
        return fsdev_edpt_xfer_fifo(rhport, ep_addr, ff, total_bytes);
    } 
    return false;
}

#endif
