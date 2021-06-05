#include <od/core.h>
#include <od/core/debug.hpp>

#include <string.h>

#include <gtest/gtest.h>

TEST(odLog, get_level_name) {
	const char* unknown_level_name = odLogLevel_get_name(OD_LOG_LEVEL_NONE);
	ASSERT_TRUE(unknown_level_name != nullptr);

	for (uint32_t level = OD_LOG_LEVEL_FIRST; level <= OD_LOG_LEVEL_LAST; level++) {

		const char* level_name = "";
		{
			odLogLevelScoped suppressLogs{OD_LOG_LEVEL_NONE};
			level_name = odLogLevel_get_name(level);
		}
		ASSERT_TRUE(level_name != nullptr);
		ASSERT_TRUE(strlen(level_name) > 0);
		ASSERT_TRUE(strcmp(level_name, unknown_level_name) != 0);
	}
}
