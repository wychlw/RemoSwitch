#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_TUD_WCH_USBIP_MERGE 1
#define CFG_TUD_WCH_USBIP_FSDEV 0
#define CFG_TUD_WCH_USBIP_USBFS 0

#if CFG_TUD_WCH_USBIP_MERGE || CFG_TUD_WCH_USBIP_FSDEV
    #define FSDEV_TUD_RHPORT 0
#endif
#if CFG_TUD_WCH_USBIP_MERGE || CFG_TUD_WCH_USBIP_USBFS
    #define USBFS_TUD_RHPORT 1
#endif

// RHPort max operational speed can defined by board.mk
#ifndef BOARD_TUD_MAX_SPEED
    #define BOARD_TUD_MAX_SPEED OPT_MODE_FULL_SPEED
#endif


#ifndef CFG_TUSB_MCU
    #define CFG_TUSB_MCU OPT_MCU_CH32V20X
#endif

#ifndef CFG_TUSB_OS
    #define CFG_TUSB_OS OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
    #define CFG_TUSB_DEBUG 3
    #define CFG_TUSB_DEBUG_PRINTF print
#endif

// Enable Device stack
#define CFG_TUD_ENABLED 1
#define CUSTOM_CFG_TUD_ENABLED 0

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED BOARD_TUD_MAX_SPEED

/* USB DMA on some MCUs can only access a specific SRAM region with restriction
 * on alignment. Tinyusb use follows macros to declare transferring memory so
 * that they can be put into those specific section. e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
    #define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
    #define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef TUP_DCD_ENDPOINT_MAX
    #define TUP_DCD_ENDPOINT_MAX 8
#endif

#ifndef CFG_TUD_ENDPOINT0_SIZE
    #define CFG_TUD_ENDPOINT0_SIZE 32
#endif

//------------- CLASS -------------//
#define CFG_TUD_HID 1
#define CFG_TUD_CDC 1
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

// buffer sizes
#define CFG_TUD_HID_EP_BUFSIZE 32
#define CFG_TUD_CDC_RX_BUFSIZE 32
#define CFG_TUD_CDC_TX_BUFSIZE 32

//------------- CONFIG -------------//
#define CFG_CDC_DEBUG_ITF 0

#ifdef __cplusplus
}
#endif

#endif
