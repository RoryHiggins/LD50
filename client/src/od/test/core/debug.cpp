#include <od/core/debug.hpp>

#include <cstring>

#include <od/test/test.hpp>

OD_TEST(odLog, get_level_name) {
	const char* unknown_level_name = odLogLevel_get_name(OD_LOG_LEVEL_NONE);
	OD_ASSERT(unknown_level_name != nullptr);

	for (int32_t level = OD_LOG_LEVEL_FIRST; level <= OD_LOG_LEVEL_LAST; level++) {

		const char* level_name = "";
		{
			odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
			level_name = odLogLevel_get_name(level);
		}
		OD_ASSERT(level_name != nullptr);
		OD_ASSERT(strcmp(level_name, unknown_level_name) != 0);
	}
}
