#pragma once

#include <od/platform/audio.h>

#include <od/core/array.hpp>

struct odAudioMixer {
	bool is_mixer_init;

	odTrivialArrayT<odAudioMixerResource*> resources;

	OD_PLATFORM_MODULE odAudioMixer();
	OD_PLATFORM_MODULE odAudioMixer(odAudioMixer&& other);
	OD_PLATFORM_MODULE odAudioMixer& operator=(odAudioMixer&& other);
	OD_PLATFORM_MODULE ~odAudioMixer();

	odAudioMixer(odAudioMixer const& other) = delete;
	odAudioMixer& operator=(const odAudioMixer& other) = delete;
};
struct odAudioMixerResource {
	odAudioMixer* mixer;

	OD_PLATFORM_MODULE odAudioMixerResource();
	OD_PLATFORM_MODULE odAudioMixerResource(odAudioMixerResource&& other);
	OD_PLATFORM_MODULE odAudioMixerResource& operator=(odAudioMixerResource&& other);
	OD_PLATFORM_MODULE ~odAudioMixerResource();

	odAudioMixerResource(odAudioMixerResource const& other) = delete;
	odAudioMixerResource& operator=(const odAudioMixerResource& other) = delete;
};
struct odAudio : odAudioMixerResource {
	void* audio_native;

	OD_PLATFORM_MODULE odAudio();
	OD_PLATFORM_MODULE odAudio(odAudio&& other);
	OD_PLATFORM_MODULE odAudio& operator=(odAudio&& other);
	OD_PLATFORM_MODULE ~odAudio();

	odAudio(odAudio const& other) = delete;
	odAudio& operator=(const odAudio& other) = delete;
};
