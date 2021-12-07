#include <od/platform/timer.h>

#include <ctime>
#include <cstring>

#include <od/core/debug.h>

void odTimer_start(odTimer* timer) {
	if (!OD_DEBUG_CHECK(timer != nullptr)) {
		return;
	}

	timer->start_time = time(nullptr);
}
float odTimer_get_elapsed_seconds(const odTimer* timer) {
	if (!OD_DEBUG_CHECK(timer != nullptr)) {
		return 0.0f;
	}

	time_t time_now = time(nullptr);
	return static_cast<float>(difftime(time_now, timer->start_time));
}
void odTimer_warn_if_exceeded(const odTimer* timer, float max_time_seconds, const struct odLogContext* log_context) {
	if (!OD_DEBUG_CHECK(timer != nullptr)) {
		return;
	}

	time_t time_now = time(nullptr);
	char time_now_str[9] = {};
	strncpy(time_now_str, ctime(&time_now) + 11, 9);

	char start_time_str[9] = {};
	strncpy(start_time_str, ctime(&timer->start_time) + 11, 9);

	float time_passed = static_cast<float>(difftime(time_now, timer->start_time));

	if (time_passed > max_time_seconds) {
		odLog_log(
			log_context,
			OD_LOG_LEVEL_WARN,
			"timer exceeded %g second(s), start_time=%.8s, time_passed~%g second(s), now=%.8s",
			static_cast<double>(max_time_seconds),
			start_time_str,
			static_cast<double>(time_passed),
			time_now_str);
	}
}
