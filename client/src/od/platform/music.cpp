#include <od/platform/music.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/platform/audio.h>

static const odMusic* odMusic_last_played = nullptr;

const odMusicPlaybackSettings* odMusicPlaybackSettings_get_defaults() {
	static const odMusicPlaybackSettings settings{
		/*loop_count*/ 0,
		/*fadein_time_ms*/ 0,
		/*start_pos_ms*/ 0,
		/*volume*/ 1.0f,
		/*is_loop_forever_enabled*/ true
	};
	return &settings;
}
bool odMusicPlaybackSettings_check_valid(const odMusicPlaybackSettings* settings) {
	if (!OD_CHECK(settings != nullptr)
		|| !OD_CHECK(settings->loop_count >= 0)
		|| !OD_CHECK(settings->start_pos_ms >= 0)
		|| !OD_CHECK(odFloat_is_normalized(settings->volume))
		|| !OD_CHECK(settings->fadein_time_ms >= 0)) {
		return false;
	}

	return true;
}

bool odMusic_init(odMusic* music) {
	if (!OD_CHECK(music != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(odAudioMixer_get_singleton()))) {
		return false;
	}

	odMusic_destroy(music);

	return music;
}
bool odMusic_init_file(odMusic* music, const char* filename) {
	if (!OD_CHECK(music != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(odAudioMixer_get_singleton()))) {
		return false;
	}

	odMusic_destroy(music);

	music->music_native = static_cast<void*>(Mix_LoadMUS(filename));
	if (!OD_CHECK(music->music_native != nullptr)) {
		OD_ERROR("Mix_LoadMUS failed, error=%s", Mix_GetError());

		return false;
	}
	return true;
}
void odMusic_destroy(odMusic* music) {
	if (!OD_CHECK(music != nullptr)) {
		return;
	}

	if (music->music_native != nullptr) {
		if (odMusic_last_played == music) {
			Mix_HaltMusic();
			odMusic_last_played = nullptr;
		}

		Mix_FreeMusic(static_cast<Mix_Music*>(music->music_native));
	}
	music->music_native = nullptr;
}
void odMusic_swap(odMusic* music1, odMusic* music2) {
	if (!OD_CHECK(music1 != nullptr)
		|| !OD_CHECK(music2 != nullptr)) {
		return;
	}

	void* music_native_swap = music1->music_native;
	music1->music_native = music2->music_native;
	music2->music_native = music_native_swap;
}
bool odMusic_check_valid(const odMusic* music) {
	if (!OD_CHECK(music != nullptr)
		|| !OD_CHECK(music->music_native != nullptr)) {
		return false;
	}

	return true;
}
bool odMusic_play(odMusic* music, const odMusicPlaybackSettings* opt_settings) {
	if (!OD_CHECK(odMusic_check_valid(music))
		|| !OD_CHECK((opt_settings == nullptr) || odMusicPlaybackSettings_check_valid(opt_settings))) {
		return 0;
	}

	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return 0;
	}

	odMusicPlaybackSettings settings = *odMusicPlaybackSettings_get_defaults();
	if (opt_settings) {
		settings = *opt_settings;
	}

	int loop_count = static_cast<int>(settings.loop_count);
	if (settings.is_loop_forever_enabled) {
		loop_count = -1;  // -1 = loop forever
	}

	int fadein_time_ms = static_cast<int>(settings.fadein_time_ms);

	double position = static_cast<double>(settings.start_pos_ms) * 1000.0;

	Mix_VolumeMusic(static_cast<int>(settings.volume * static_cast<float>(MIX_MAX_VOLUME)));

	int result = Mix_FadeInMusicPos(
		static_cast<Mix_Music*>(music->music_native), loop_count, fadein_time_ms, position);

	if (!OD_CHECK(result == 0)) {
		OD_WARN("Mix_FadeInMusicPos() failed, error=%s", Mix_GetError());
		return false;
	}
	if (!OD_CHECK(Mix_PlayingMusic() == 1)) {
		OD_ERROR("Mix_FadeInMusicPos() failed, error=%s", Mix_GetError());
		return false;
	}

	odMusic_last_played = music;

	return true;
}
void odMusic_stop() {
	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return;
	}

	odMusic_last_played = nullptr;

	OD_DISCARD(OD_CHECK(Mix_HaltMusic() == 0));
}
void odMusic_pause() {
	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return;
	}

	Mix_PauseMusic();
}
void odMusic_resume() {
	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return;
	}

	Mix_ResumeMusic();
}
bool odMusic_is_playing() {
	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return false;
	}

	return Mix_PlayingMusic();
}
bool odMusic_is_paused() {
	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return false;
	}

	// intentionally looking at both, to hide some unintuitive SDL_mixer behaviour:
	// from Mix_PausedMusic: "Does not check if the music was been halted after it was paused"
	// from Mix_PlayingMusic: "Does not check if the channel has been paused."
	return Mix_PlayingMusic() && Mix_PausedMusic();
}

odMusic::odMusic()
: music_native{nullptr} {
}
odMusic::odMusic(odMusic&& other)
: odMusic{} {
	odMusic_swap(this, &other);
}
odMusic& odMusic::operator=(odMusic&& other) {
	odMusic_swap(this, &other);
	return *this;
}
odMusic::~odMusic() {
	odMusic_destroy(this);
}
