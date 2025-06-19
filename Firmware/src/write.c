#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "board.h"
#include "ch32v20x.h"
#include "queued_task.h"
#include "write.h"

bool print_acm_inited = false;

int debug_uart_write(void const *buf, int len) {
    const char *bufc = (const char *) buf;
    for (int i = 0; i < len; i++) {
        while (USART_GetFlagStatus(HOST_UART_DEV, USART_FLAG_TC) == RESET);
        USART_SendData(HOST_UART_DEV, *bufc++);
    }

    return len;
}

static char acm_send_buf[ACM_RING_BUF_SZ];
static uint32_t acm_send_head = 0;
static uint32_t acm_send_tail = 0;
static void acm_send_cb() {
    if (acm_send_head == acm_send_tail) {
        return;
    }
    if (acm_send_head < acm_send_tail) {
        uint32_t written = tud_cdc_n_write(CFG_CDC_DEBUG_ITF, acm_send_buf + acm_send_head, acm_send_tail - acm_send_head);
        tud_cdc_n_write_flush(CFG_CDC_DEBUG_ITF);
        acm_send_head += written;
    } else {
        uint32_t written = tud_cdc_n_write(CFG_CDC_DEBUG_ITF, acm_send_buf + acm_send_head, ACM_RING_BUF_SZ - acm_send_head);
        tud_cdc_n_write_flush(CFG_CDC_DEBUG_ITF);
        acm_send_head = acm_send_head + written;
    }
    if (acm_send_head >= ACM_RING_BUF_SZ) {
        acm_send_head -= ACM_RING_BUF_SZ;
    }
}

static void acm_buf_append(const char *str, uint32_t len) {
    uint32_t remain = ACM_RING_BUF_SZ - acm_send_tail + acm_send_head;
    if (remain < len) {
        return;
    }
    if (acm_send_tail < acm_send_head) {
        strncpy(acm_send_buf + acm_send_tail, str, len);
        acm_send_tail += len;
    } else if (acm_send_tail + len < ACM_RING_BUF_SZ) {
        strncpy(acm_send_buf + acm_send_tail, str, len);
        acm_send_tail += len;
    } else {
        strncpy(acm_send_buf + acm_send_tail, str, ACM_RING_BUF_SZ - acm_send_tail);
        strncpy(acm_send_buf, str + ACM_RING_BUF_SZ - acm_send_tail, len - ACM_RING_BUF_SZ + acm_send_tail);
        acm_send_tail = len - ACM_RING_BUF_SZ + acm_send_tail;
    }
}

static int vprint(const char *fmt, va_list args) {
    char buf[256];
    uint32_t len = vsnprintf(buf, sizeof(buf), fmt, args);

    debug_uart_write(buf, len);

    if (print_acm_inited) {
        acm_buf_append(buf, len);
    }

    return len;
}

int print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int len = vprint(fmt, args);

    va_end(args);

    return len;
}

void print_init() {
#if defined(CH32V203C8T6)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    GPIO_InitTypeDef usart_init = {
        .GPIO_Pin = HOST_UART_RX_PIN | HOST_UART_TX_PIN,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode = GPIO_Mode_AF_PP,
    };
    GPIO_Init(GPIOA, &usart_init);
#endif
#if defined(CH32V203G6U6)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    GPIO_InitTypeDef usart_init = {
        .GPIO_Pin = HOST_UART_RX_PIN | HOST_UART_TX_PIN,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode = GPIO_Mode_AF_PP,
    };
    GPIO_Init(GPIOA, &usart_init);
#endif

    USART_InitTypeDef usart = {
        .USART_BaudRate = 115200,
        .USART_WordLength = USART_WordLength_8b,
        .USART_StopBits = USART_StopBits_1,
        .USART_Parity = USART_Parity_No,
        .USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
        .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    };
    USART_Init(HOST_UART_DEV, &usart);
    USART_Cmd(HOST_UART_DEV, ENABLE);

    queued_task_add(acm_send_cb, 50, true);
}
