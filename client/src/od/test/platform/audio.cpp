#include <od/platform/audio.hpp>

#include <cstring>

#include <od/test/test.hpp>

OD_TEST_FILTERED(odTest_odAudioMixer_init_destroy, OD_TEST_FILTER_SLOW) {
	odAudioMixer mixer;
	OD_ASSERT(odAudioMixer_init(&mixer));
	OD_ASSERT(odAudioMixer_init(&mixer));  // test double init

	odAudioMixer_destroy(&mixer);
	odAudioMixer_destroy(&mixer);  // test double destroy
}
OD_TEST_FILTERED(odTest_odAudioMixer_swap, OD_TEST_FILTER_SLOW) {
	odAudioMixer mixer1;
	odAudioMixer mixer2;
	OD_ASSERT(odAudioMixer_init(&mixer1));
	OD_ASSERT(odAudioMixer_init(&mixer2));

	odAudioMixer_swap(&mixer1, &mixer2);
}
OD_TEST_SUITE(
	odTestSuite_odAudio,
	odTest_odAudioMixer_init_destroy,
	odTest_odAudioMixer_swap,
)
