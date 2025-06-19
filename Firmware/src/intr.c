#include <ch32v20x.h>

#include "board.h"
#include "bsp/board_api.h"
#include "intr.h"
#include "util.h"

void intr_init(void) {
    TIM_TimeBaseInitTypeDef tb = {0};
    NVIC_InitTypeDef nvic = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    tb.TIM_Period = 1000 - 1;
    tb.TIM_Prescaler = clocks.PCLK1_Frequency / 1000000 - 1;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &tb);

    nvic.NVIC_IRQChannel = TIM4_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

void TickMS_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
volatile uint32_t system_ticks = 0;
void TickMS_Handler(void) {
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        system_ticks++;
    }
}

void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void) {
    SysTick->SR = 0;
}

void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt));
void USB_LP_CAN1_RX0_IRQHandler(void) {
    tud_int_handler(0);
}

void USB_HP_CAN1_TX_IRQHandler(void) __attribute__((interrupt));
void USB_HP_CAN1_TX_IRQHandler(void) {
    tud_int_handler(0);
}

void USBWakeUp_IRQHandler(void) __attribute__((interrupt));
void USBWakeUp_IRQHandler(void) {
    tud_int_handler(0);
}

void USBHD_IRQHandler(void) __attribute__((interrupt));
void USBHD_IRQHandler(void) {
    tud_int_handler(1);
}

void USBHDWakeUp_IRQHandler(void) __attribute__((interrupt));
void USBHDWakeUp_IRQHandler(void) {
    tud_int_handler(1);
}