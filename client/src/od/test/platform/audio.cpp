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
OD_TEST_FILTERED(odTest_odAudio_init_destroy, OD_TEST_FILTER_SLOW) {
	odAudioMixer mixer;
	OD_ASSERT(odAudioMixer_init(&mixer));

	odAudio audio;
	OD_ASSERT(odAudio_init(&audio, &mixer));
	OD_ASSERT(odAudio_init(&audio, &mixer));  // test double init

	odAudio_destroy(&audio);
	odAudio_destroy(&audio);  // test double destroy
}
OD_TEST_FILTERED(odTest_odAudio_init_destroy_after_mixer, OD_TEST_FILTER_SLOW) {
	odAudioMixer mixer;
	OD_ASSERT(odAudioMixer_init(&mixer));

	odAudio audio;
	OD_ASSERT(odAudio_init(&audio, &mixer));

	odAudioMixer_destroy(&mixer);

	odAudio_destroy(&audio);
}
OD_TEST_FILTERED(odTest_odAudio_init_wav, OD_TEST_FILTER_SLOW) {
	odAudioMixer mixer;
	OD_ASSERT(odAudioMixer_init(&mixer));

	const uint8_t wav_two_samples_22050hz_u16[] = {
		0x52, 0x49, 0x46, 0x46, 0x28, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
		0x66, 0x6d, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x22, 0x56, 0x00, 0x00, 0x44, 0xac, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00,
		0x64, 0x61, 0x74, 0x61, 0x04, 0x00, 0x00, 0x00, 0x66, 0x66, 0x67, 0x66
	};
	const int32_t wav_size = sizeof(wav_two_samples_22050hz_u16);

	odAudio audio;
	OD_ASSERT(odAudio_init_wav(&audio, &mixer, static_cast<const void*>(wav_two_samples_22050hz_u16), wav_size));
}
OD_TEST_SUITE(
	odTestSuite_odAudio,
	odTest_odAudioMixer_init_destroy,
	odTest_odAudioMixer_swap,
	odTest_odAudio_init_destroy,
	odTest_odAudio_init_destroy_after_mixer,
	odTest_odAudio_init_wav,
)
