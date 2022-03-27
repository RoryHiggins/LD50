#include <od/platform/music.hpp>

#include <cstring>

#include <od/platform/timer.h>
#include <od/platform/sdl.h>
#include <od/test/test.hpp>

static OD_NO_DISCARD bool
odTest_odMusic_init_default(odMusic* music);
static OD_NO_DISCARD bool
odTest_odMusic_wait_until_complete(float max_time_sec);
static const odMusicPlaybackSettings*
odTest_odMusicPlaybackSettings_get_defaults();

bool odTest_odMusic_init_default(odMusic* music) {
	return odMusic_init_file(music, "examples/engine_test/data/100ms_sine_440hz_22050hz_s16.ogg");
}
bool odTest_odMusic_wait_until_complete(float max_time_sec) {
	odTimer timer;
	odTimer_start(&timer);

	while (odMusic_is_playing()) {
		int32_t sleep_ms = 1;
		odSDL_sleep(sleep_ms);

		if (odTimer_get_elapsed_seconds(&timer) > max_time_sec) {
			return false;
		}
	}

	return true;
}
const odMusicPlaybackSettings* odTest_odMusicPlaybackSettings_get_defaults() {
	static const odMusicPlaybackSettings settings{
		/*loop_count*/ 0,
		/*fadein_time_ms*/ 0,
		/*start_pos_ms*/ 0,
		/*volume*/ 0.0f,
		/*is_loop_forever_enabled*/ false,
	};
	return &settings;
}

OD_TEST_FILTERED(odTest_odMusic_init_destroy, OD_TEST_FILTER_SLOW) {
	odMusic music;
	OD_ASSERT(odMusic_init(&music));
	OD_ASSERT(odMusic_init(&music));  // test double init

	odMusic_destroy(&music);
	odMusic_destroy(&music);  // test double destroy
}

OD_TEST_FILTERED(odTest_odMusic_init_file, OD_TEST_FILTER_SLOW) {
	odMusic music;
	OD_ASSERT(odTest_odMusic_init_default(&music));
}
OD_TEST_FILTERED(odTest_odMusic_play_stop, OD_TEST_FILTER_SLOW) {
	odMusic music;
	OD_ASSERT(odTest_odMusic_init_default(&music));
	
	odMusic_stop();
	OD_ASSERT(!odMusic_is_playing());
	OD_ASSERT(odMusic_play(&music, odTest_odMusicPlaybackSettings_get_defaults()));
	OD_ASSERT(odMusic_is_playing());
	OD_ASSERT(odTest_odMusic_wait_until_complete(0.5f));

	odMusic_stop();
}
OD_TEST_FILTERED(odTest_odMusic_play_destroy, OD_TEST_FILTER_SLOW) {
	odMusic music;
	OD_ASSERT(odTest_odMusic_init_default(&music));
	
	odMusic_stop();
	OD_ASSERT(odMusic_play(&music, odTest_odMusicPlaybackSettings_get_defaults()));
	OD_ASSERT(odMusic_is_playing());

	odMusic_destroy(&music);
	OD_ASSERT(!odMusic_is_playing());

	odMusic_stop();
}
OD_TEST_FILTERED(odTest_odMusic_play_pause_resume_stop, OD_TEST_FILTER_SLOW) {
	odMusic music;
	OD_ASSERT(odTest_odMusic_init_default(&music));
	
	odMusic_stop();
	OD_ASSERT(odMusic_play(&music, odTest_odMusicPlaybackSettings_get_defaults()));
	OD_ASSERT(odMusic_is_playing());
	OD_ASSERT(!odMusic_is_paused());

	odMusic_pause();
	OD_ASSERT(odMusic_is_playing());
	OD_ASSERT(odMusic_is_paused());

	odMusic_resume();
	OD_ASSERT(odMusic_is_playing());
	OD_ASSERT(!odMusic_is_paused());

	odMusic_stop();
	OD_ASSERT(!odMusic_is_playing());
	OD_ASSERT(!odMusic_is_paused());

	OD_ASSERT(odTest_odMusic_wait_until_complete(0.1f));

	odMusic_stop();
}

OD_TEST_SUITE(
	odTestSuite_odMusic,
	odTest_odMusic_init_destroy,
	odTest_odMusic_init_file,
	odTest_odMusic_play_stop,
	odTest_odMusic_play_destroy,
	odTest_odMusic_play_pause_resume_stop,
)
