#pragma once

#include <od/platform/module.h>

struct odMusic;

struct odMusicPlaybackSettings {
	int32_t loop_count;
	int32_t fadein_time_ms;
	int32_t start_pos_ms;
	float volume;  // 0..1

	bool is_loop_forever_enabled;
};

OD_API_C OD_PLATFORM_MODULE const struct odMusicPlaybackSettings*
odMusicPlaybackSettings_get_defaults(void);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odMusicPlaybackSettings_check_valid(const struct odMusicPlaybackSettings* settings);

OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odMusic_init(struct odMusic* music);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odMusic_init_file(struct odMusic* music, const char* filename);
OD_API_C OD_PLATFORM_MODULE void
odMusic_destroy(struct odMusic* music);
OD_API_C OD_PLATFORM_MODULE void
odMusic_swap(struct odMusic* music1, struct odMusic* music2);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odMusic_check_valid(const struct odMusic* music);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odMusic_play(struct odMusic* music, const struct odMusicPlaybackSettings* opt_settings);
OD_API_C OD_PLATFORM_MODULE void
odMusic_stop(void);
OD_API_C OD_PLATFORM_MODULE void
odMusic_pause(void);
OD_API_C OD_PLATFORM_MODULE void
odMusic_resume(void);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odMusic_is_playing(void);
OD_API_C OD_PLATFORM_MODULE OD_NO_DISCARD bool
odMusic_is_paused(void);
