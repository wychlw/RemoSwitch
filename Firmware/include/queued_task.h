#ifndef __QUEUED_TASK_H
#define __QUEUED_TASK_H

#include "ch32v20x.h"

#define CFG_TASK_QUEUE_SIZE 16
#define CFG_TASK_INTERVAL_MS 10
#define CFG_TASK_TIMER TIM3 // If change, remember to change in queued_task.c

void queued_task_init();
void queued_task_add(void (*callback)(void), int32_t time, bool repeat);

#endif