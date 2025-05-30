#ifndef __WRITE_C
#define __WRITE_C

#include <stdbool.h>

#define ACM_RING_BUF_SZ 512

extern bool print_acm_inited;

int print(const char *fmt, ...);
int scan(const char *fmt, ...);

void print_init();

#include "config.h"

#if LOG_LEVEL >= 1
#define error(fmt, ...) print("[ERROR]  " __FILE__ ":%d:\t" fmt, __LINE__, ##__VA_ARGS__)
#else
#define error(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= 2
#define warn(fmt, ...) print("[WARN]    " __FILE__ ":%d:\t" fmt, __LINE__, ##__VA_ARGS__)
#else
#define warn(fmt, ...) ((void)0)
#endif

#if LOG_LEVEL >= 3
#define info(fmt, ...) print("[INFO]    " __FILE__ ":%d:\t" fmt, __LINE__, ##__VA_ARGS__)
#else
#define info(fmt, ...) ((void)0)
#endif

#endif