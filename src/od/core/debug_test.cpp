#include <od/core.h>
#include <od/core/debug.hpp>

#include <string.h>

#include <gtest/gtest.h>

TEST(odLog, get_level_name) {
	const char* unknown_level_name = odLogLevel_get_name(OD_LOG_LEVEL_NONE);
	ASSERT_NE(unknown_level_name, nullptr);

	for (uint32_t level = OD_LOG_LEVEL_FIRST; level <= OD_LOG_LEVEL_LAST; level++) {

		const char* level_name = "";
		{
			odLogLevelScoped suppress_logs{OD_LOG_LEVEL_NONE};
			level_name = odLogLevel_get_name(level);
		}
		ASSERT_NE(level_name, nullptr);
		ASSERT_GT(strlen(level_name), 0);
		ASSERT_NE(strcmp(level_name, unknown_level_name), 0);
	}
}
