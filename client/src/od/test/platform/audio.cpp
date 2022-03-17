#include <od/platform/audio.hpp>

#include <cstring>

#include <od/platform/timer.h>
#include <od/platform/sdl.h>
#include <od/test/test.hpp>

static OD_NO_DISCARD bool
odTest_odAudio_loop_until_playback_complete(odAudioPlaybackId playback_id, float max_time_sec);
static OD_NO_DISCARD bool
odTest_odAudio_init_sine(odAudio* audio);
static const odAudioPlaybackSettings*
odTest_odAudioAudioPlaybackSettings_get_defaults();

bool odTest_odAudio_loop_until_playback_complete(odAudioPlaybackId playback_id, float max_time_sec) {
	odTimer timer;
	odTimer_start(&timer);

	while (odAudio_is_playing(playback_id)) {
		int32_t sleep_ms = 1;
		odSDL_sleep(sleep_ms);

		if (odTimer_get_elapsed_seconds(&timer) > max_time_sec) {
			return false;
		}
	}

	return true;
}
bool odTest_odAudio_init_sine(odAudio* audio) {
	const uint8_t wav_sine_22050hz_s16[] = {
		0x52, 0x49, 0x46, 0x46, 0x88, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
		0x66, 0x6d, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x22, 0x56, 0x00, 0x00, 0x44, 0xac, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00,
		0x64, 0x61, 0x74, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 0x0c,
		0x67, 0x19, 0x9f, 0x25, 0x39, 0x31, 0x14, 0x3c, 0xf8, 0x45, 0xc4, 0x4e,
		0x57, 0x56, 0x89, 0x5c, 0x4e, 0x61, 0x86, 0x64, 0x2d, 0x66, 0x38, 0x66,
		0xab, 0x64, 0x84, 0x61, 0xdc, 0x5c, 0xb7, 0x56, 0x3f, 0x4f, 0x7d, 0x46,
		0xac, 0x3c, 0xdc, 0x31, 0x4c, 0x26, 0x1d, 0x1a, 0x88, 0x0d, 0xb9, 0x00,
		0xed, 0xf3, 0x4b, 0xe7, 0x13, 0xdb, 0x67, 0xcf, 0x87, 0xc4, 0x8f, 0xba,
		0xb3, 0xb1, 0x11, 0xaa, 0xc4, 0xa3, 0xf1, 0x9e, 0x9c, 0x9b, 0xe0, 0x99,
		0xbe, 0x99, 0x34, 0x9b, 0x43, 0x9e, 0xd6, 0xa2, 0xe7, 0xa8, 0x4b, 0xb0,
		0xfb, 0xb8, 0xc0, 0xc2, 0x7e, 0xcd, 0x0b, 0xd9, 0x2b, 0xe5, 0xc2, 0xf1
	};
	const int32_t wav_size = sizeof(wav_sine_22050hz_s16);
	return OD_CHECK(odAudio_init_wav(audio, wav_sine_22050hz_s16, wav_size));
}
const odAudioPlaybackSettings* odTest_odAudioAudioPlaybackSettings_get_defaults() {
	static const odAudioPlaybackSettings settings{
		/*loop_count*/ 0,
		/*cutoff_time_ms*/ 10,
		/*fade_in_time_ms*/ 0,
		/*volume*/ 0.0f,
		/*is_loop_forever_enabled*/ true,
		/*is_cutoff_time_enabled*/ true,
	};
	return &settings;
}

OD_TEST_FILTERED(odTest_odAudio_init_destroy, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odAudio_init(&audio));
	OD_ASSERT(odAudio_init(&audio));  // test double init

	odAudio_destroy(&audio);
	odAudio_destroy(&audio);  // test double destroy
}
OD_TEST_FILTERED(odTest_odAudio_init_wav, OD_TEST_FILTER_SLOW) {
	const uint8_t wav_two_samples_22050hz_s16[] = {
		0x52, 0x49, 0x46, 0x46, 0x28, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
		0x66, 0x6d, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x22, 0x56, 0x00, 0x00, 0x44, 0xac, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00,
		0x64, 0x61, 0x74, 0x61, 0x04, 0x00, 0x00, 0x00, 0x66, 0x66, 0x67, 0x66
	};
	const int32_t wav_size = sizeof(wav_two_samples_22050hz_s16);

	odAudio audio;
	OD_ASSERT(odAudio_init_wav(&audio, static_cast<const void*>(wav_two_samples_22050hz_s16), wav_size));
}
OD_TEST_FILTERED(odTest_odAudio_set_volume, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odTest_odAudio_init_sine(&audio));

	OD_ASSERT(odAudio_set_volume(&audio, 1.0f));
	OD_ASSERT(odAudio_set_volume(&audio, 0.5f));
	OD_ASSERT(odAudio_set_volume(&audio, 0.0f));
}
OD_TEST_FILTERED(odTest_odAudio_set_volume_out_of_range_fails, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odTest_odAudio_init_sine(&audio));

	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		OD_ASSERT(!odAudio_set_volume(&audio, -0.01f));
		OD_ASSERT(!odAudio_set_volume(&audio, 1.01f));
	}
}
OD_TEST_FILTERED(odTest_odAudio_play, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odTest_odAudio_init_sine(&audio));

	odAudioPlaybackId playback_id = odAudio_play(&audio, odTest_odAudioAudioPlaybackSettings_get_defaults());
	OD_ASSERT(odAudioPlaybackId_check_valid(playback_id));

	OD_ASSERT(odTest_odAudio_loop_until_playback_complete(playback_id, 1.0f));

	OD_ASSERT(odAudio_stop_all());
}
OD_TEST_FILTERED(odTest_odAudio_play_simultaneous, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odTest_odAudio_init_sine(&audio));

	const int32_t playback_ids_count = OD_AUDIO_MIXER_CHANNELS;
	odAudioPlaybackId playback_ids[playback_ids_count] = {};
	for (int32_t i = 0; i < playback_ids_count; i++) {
		playback_ids[i] = odAudio_play(&audio, odTest_odAudioAudioPlaybackSettings_get_defaults());
		OD_ASSERT(odAudioPlaybackId_check_valid(playback_ids[i]));
		OD_ASSERT(odAudio_is_playing(playback_ids[i]));
	}

	OD_ASSERT(odTest_odAudio_loop_until_playback_complete(playback_ids[playback_ids_count - 1], 1.0f));
	for (int32_t i = 0; i < playback_ids_count; i++) {
		OD_ASSERT(odTest_odAudio_loop_until_playback_complete(playback_ids[i], 0.1f));
		OD_ASSERT(!odAudio_is_playing(playback_ids[i]));
	}

	OD_ASSERT(odAudio_stop_all());
}
OD_TEST_FILTERED(odTest_odAudio_play_simultaneous_above_max_fails, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odTest_odAudio_init_sine(&audio));

	const int32_t playback_ids_count = OD_AUDIO_MIXER_CHANNELS;
	odAudioPlaybackId playback_ids[playback_ids_count] = {};
	for (int32_t i = 0; i < playback_ids_count; i++) {
		playback_ids[i] = odAudio_play(&audio, odTest_odAudioAudioPlaybackSettings_get_defaults());
		OD_ASSERT(odAudioPlaybackId_check_valid(playback_ids[i]));
		OD_ASSERT(odAudio_is_playing(playback_ids[i]));
	}

	// 1 more than OD_AUDIO_MIXER_CHANNELS should fail
	{
		odLogLevelScoped suppress_errors{OD_LOG_LEVEL_FATAL};
		odAudioPlaybackId invalid_playback_id = odAudio_play(
			&audio, odTest_odAudioAudioPlaybackSettings_get_defaults());
		OD_ASSERT(!odAudioPlaybackId_check_valid(invalid_playback_id));
	}

	OD_ASSERT(odAudio_stop_all());
}
OD_TEST_FILTERED(odTest_odAudio_play_destroy, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odTest_odAudio_init_sine(&audio));

	odAudioPlaybackId playback_id = odAudio_play(&audio, odTest_odAudioAudioPlaybackSettings_get_defaults());
	OD_ASSERT(odAudioPlaybackId_check_valid(playback_id));
	OD_ASSERT(odAudio_is_playing(playback_id));

	odAudio_destroy(&audio);

	OD_ASSERT(!odAudio_is_playing(playback_id));

	OD_ASSERT(odAudio_stop_all());
}
OD_TEST_FILTERED(odTest_odAudio_play_stop, OD_TEST_FILTER_SLOW) {
	odAudio audio;
	OD_ASSERT(odTest_odAudio_init_sine(&audio));

	odAudioPlaybackId playback_id = odAudio_play(&audio, odTest_odAudioAudioPlaybackSettings_get_defaults());
	OD_ASSERT(odAudioPlaybackId_check_valid(playback_id));

	OD_ASSERT(odAudio_stop(playback_id));
	OD_ASSERT(!odAudio_is_playing(playback_id));

	OD_ASSERT(odAudio_stop(playback_id));  // test re-stop
	OD_ASSERT(!odAudio_is_playing(playback_id));

	OD_ASSERT(odAudio_stop_all());
}
// TODO test odAudio_destroy during playback
OD_TEST_SUITE(
	odTestSuite_odAudio,
	odTest_odAudio_init_destroy,
	odTest_odAudio_init_wav,
	odTest_odAudio_set_volume,
	odTest_odAudio_set_volume_out_of_range_fails,
	odTest_odAudio_play,
	odTest_odAudio_play_simultaneous,
	odTest_odAudio_play_simultaneous_above_max_fails,
	odTest_odAudio_play_destroy,
	odTest_odAudio_play_stop,
)
