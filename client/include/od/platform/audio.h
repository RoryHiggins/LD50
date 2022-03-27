#pragma once

#include <od/platform/module.h>

#define OD_AUDIO_MIXER_CHANNELS 64
#define OD_AUDIO_PLAYBACK_ID_NO_CHANNELS -1

typedef int32_t odAudioPlaybackId;

struct odAudioMixer;
struct odAudio;

struct odAudioPlaybackSettings {
	int32_t loop_count;
	int32_t cutoff_time_ms;
	int32_t fadein_time_ms;
	float volume;  // 0..1

	bool is_loop_forever_enabled;
	bool is_cutoff_time_enabled;
};

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudioPlaybackId_check_valid(odAudioPlaybackId playback_id);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudioMixer_check_valid(const struct odAudioMixer* mixer);
OD_API_C OD_PLATFORM_MODULE struct odAudioMixer*
odAudioMixer_get_singleton(void);

OD_API_C OD_PLATFORM_MODULE const struct odAudioPlaybackSettings*
odAudioPlaybackSettings_get_defaults(void);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudioPlaybackSettings_check_valid(const struct odAudioPlaybackSettings* settings);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_init(struct odAudio* audio);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_init_wav(struct odAudio* audio, const void* src_wav, int32_t src_wav_size);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_init_wav_file(struct odAudio* audio, const char* filename);
OD_API_C OD_PLATFORM_MODULE void
odAudio_destroy(struct odAudio* audio);
OD_API_C OD_PLATFORM_MODULE void
odAudio_swap(struct odAudio* audio1, struct odAudio* audio2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_check_valid(const struct odAudio* audio);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_set_volume(struct odAudio* audio, float volume);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD odAudioPlaybackId
odAudio_play(struct odAudio* audio, const struct odAudioPlaybackSettings* opt_settings);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_stop(odAudioPlaybackId playback_id);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_stop_all(void);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odAudio_is_playing(odAudioPlaybackId playback_id);
