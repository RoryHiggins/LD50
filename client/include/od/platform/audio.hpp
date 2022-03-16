#pragma once

#include <od/platform/audio.h>

struct odAudioMixer {
	bool is_mixer_init;

	OD_PLATFORM_MODULE odAudioMixer();
	OD_PLATFORM_MODULE odAudioMixer(odAudioMixer&& other);
	OD_PLATFORM_MODULE odAudioMixer& operator=(odAudioMixer&& other);
	OD_PLATFORM_MODULE ~odAudioMixer();

	odAudioMixer(odAudioMixer const& other) = delete;
	odAudioMixer& operator=(const odAudioMixer& other) = delete;
};
