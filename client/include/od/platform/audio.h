#pragma once

#include <od/platform/module.h>

typedef uint32_t odAudioId;

struct odAudio;
struct odAudioMixer;

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_init(struct odAudio* audio);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD odAudioId
odAudio_init_from_wav_file(struct odAudio* audio, const char* filename);
OD_API_C OD_PLATFORM_MODULE void
odAudio_destroy(struct odAudio* audio);
OD_API_C OD_PLATFORM_MODULE void
odAudio_swap(struct odAudio* audio1, struct odAudio* audio2);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudioMixer_init(struct odAudioMixer* mixer);
OD_API_C OD_PLATFORM_MODULE void
odAudioMixer_destroy(struct odAudioMixer* mixer);
OD_API_C OD_PLATFORM_MODULE void
odAudioMixer_swap(struct odAudioMixer* mixer1, struct odAudioMixer* mixer2);
