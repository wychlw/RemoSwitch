#ifndef BOARD_H_
#define BOARD_H_

#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLINK_PORT      GPIOA
#define BLINK_PIN       GPIO_Pin_15

#define MUX_EN_PORT         GPIOA
#define MUX_EN_PIN          GPIO_Pin_3

#define SEL0_PORT           GPIOA
#define SEL0_PIN            GPIO_Pin_4

#define SEL1_PORT           GPIOA
#define SEL1_PIN            GPIO_Pin_5

#define UP_CHO_PORT         GPIOA
#define UP_CHO_PIN          GPIO_Pin_7

#define SD_HOST_LED_PORT    GPIOB
#define SD_HOST_LED_PIN     GPIO_Pin_13

#define SD_TS_LED_PORT      GPIOB
#define SD_TS_LED_PIN       GPIO_Pin_12

#define HOST_UART_DEV       USART1
#define HOST_UART_TX_PIN    GPIO_Pin_9
#define HOST_UART_RX_PIN    GPIO_Pin_10

#define TF_VCC_EN_PORT      GPIOA
#define TF_VCC_EN_PIN       GPIO_Pin_6

#define DOWN_VCC_EN_PORT    GPIOB
#define DOWN_VCC_EN_PIN     GPIO_Pin_0

#define UP_VCC_EN_PORT      GPIOB
#define UP_VCC_EN_PIN       GPIO_Pin_1

#define TickMS_TIM          TIM4

// end

#ifdef __cplusplus
}
#endif

#endif
