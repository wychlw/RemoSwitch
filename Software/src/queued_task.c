#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "queued_task.h"

#include "write.h"
#include "util.h"

typedef struct {
    bool repeat;
    bool enabled;
    int32_t initial_time;
    int32_t remain_time;
    void (*callback)(void);
} queued_task_t;

static queued_task_t queued_tasks[CFG_TASK_QUEUE_SIZE];

void queued_task_add(void (*callback)(void), int32_t time, bool repeat) {
    for (uint16_t i = 0; i < CFG_TASK_QUEUE_SIZE; i++) {
        if (!queued_tasks[i].enabled) {
            queued_tasks[i].callback = callback;
            queued_tasks[i].initial_time = time;
            queued_tasks[i].remain_time = time;
            queued_tasks[i].repeat = repeat;
            queued_tasks[i].enabled = true;
            return;
        }
    }

    error("Task queue is full!\r\n");
}

static void queued_task_interval_cb() {
    for (uint16_t i = 0; i < CFG_TASK_QUEUE_SIZE; i++) {
        if (queued_tasks[i].enabled) {
            if (queued_tasks[i].remain_time <= 0) {
                queued_tasks[i].callback();
                if (queued_tasks[i].repeat) {
                    queued_tasks[i].remain_time = queued_tasks[i].initial_time;
                } else {
                    queued_tasks[i].enabled = false;
                }
            } else {
                queued_tasks[i].remain_time -= CFG_TASK_INTERVAL_MS;
            }
        }
    }
}

void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler() {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        queued_task_interval_cb();
    }
}

void queued_task_init() {
    for (uint16_t i = 0; i < CFG_TASK_QUEUE_SIZE; i++) {
        queued_tasks[i].enabled = false;
    }

    TIM_TimeBaseInitTypeDef tb = {0};
    NVIC_InitTypeDef nvic = {0};

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    tb.TIM_Period = CFG_TASK_INTERVAL_MS * 1000 - 1;
    tb.TIM_Prescaler = clocks.PCLK1_Frequency / 1000000 - 1;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tb);

    nvic.NVIC_IRQChannel = TIM3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}
