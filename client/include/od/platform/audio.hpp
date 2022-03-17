#pragma once

#include <od/platform/audio.h>

#include <od/core/array.hpp>

struct odAudio {
	void* audio_native;

	OD_PLATFORM_MODULE odAudio();
	OD_PLATFORM_MODULE odAudio(odAudio&& other);
	OD_PLATFORM_MODULE odAudio& operator=(odAudio&& other);
	OD_PLATFORM_MODULE ~odAudio();

	odAudio(odAudio const& other) = delete;
	odAudio& operator=(const odAudio& other) = delete;
};
