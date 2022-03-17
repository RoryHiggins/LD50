#pragma once

#include <od/platform/module.h>

struct odAudioMixer;
struct odAudioMixerResource;
struct odAudio;

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudioMixer_init(struct odAudioMixer* mixer);
OD_API_C OD_PLATFORM_MODULE void
odAudioMixer_destroy(struct odAudioMixer* mixer);
OD_API_C OD_PLATFORM_MODULE void
odAudioMixer_swap(struct odAudioMixer* mixer1, struct odAudioMixer* mixer2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudioMixer_check_valid(const struct odAudioMixer* mixer);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudioMixerResource_init(struct odAudioMixerResource* resource, struct odAudioMixer* opt_mixer);
OD_API_C OD_PLATFORM_MODULE void
odAudioMixerResource_destroy(struct odAudioMixerResource* resource);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_init(struct odAudio* audio, struct odAudioMixer* mixer);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_init_wav(struct odAudio* audio, struct odAudioMixer* mixer, const void* src_wav, int32_t src_wav_size);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_init_wav_file(struct odAudio* audio, struct odAudioMixer* mixer, const char* filename);
OD_API_C OD_PLATFORM_MODULE void
odAudio_destroy(struct odAudio* audio);
OD_API_C OD_PLATFORM_MODULE void
odAudio_swap(struct odAudio* audio1, struct odAudio* audio2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_check_valid(const struct odAudio* audio);
