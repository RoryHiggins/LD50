#pragma once

#include <od/platform/module.h>

#include <ctime>

#define OD_TIMER_WARN_IF_EXCEEDED(TIMER, MAX_TIME_SEC) \
	odTimer_warn_if_exceeded(TIMER, MAX_TIME_SEC, OD_LOG_SET_CONTEXT())

struct odLogContext;

// low-resolution timer
struct odTimer {
	time_t start_time;
};

OD_API_C OD_PLATFORM_MODULE void
odTimer_start(struct odTimer* timer);
OD_API_C OD_PLATFORM_MODULE float
odTimer_get_elapsed_seconds(const struct odTimer* timer);
OD_API_C OD_PLATFORM_MODULE void
odTimer_warn_if_exceeded(const struct odTimer* timer, float max_time_sec, const struct odLogContext* log_context);
