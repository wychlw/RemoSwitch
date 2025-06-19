#include "bsp/board_api.h"
#include "ch32v20x.h"
#include "tusb.h"

#include "queued_task.h"
#include "util.h"
#include "write.h"

static void welcome_print_cb() {
    info("USB device connected\r\n");
    info("Welcome to tf mux debug console\r\n");
}

static void welcome_cb() {
    print_acm_inited = true;

    queued_task_add(welcome_print_cb, 1000, false);
}

void tud_mount_cb(void) {
    info("USB Mounted\r\n");

    queued_task_add(welcome_cb, 500, false);
}

void tud_umount_cb(void) {
    print_acm_inited = false;
    info("USB Unmounted...\r\n");
}

void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
}

void tud_resume_cb(void) {
}