#include <od/platform/audio.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <od/core/debug.h>
#include <od/core/math.h>
#include <od/platform/sdl.h>
#include <od/platform/file.h>

#define OD_AUDIO_PLAYBACK_CHANNEL_BITS 8
#define OD_AUDIO_PLAYBACK_CHANNEL_MASK 0xFF
#define OD_AUDIO_PLAYBACK_GENERATION_MASK 0xFF00

struct odAudioMixer;

static OD_NO_DISCARD bool
odAudioMixer_check_valid(const odAudioMixer* mixer);
static odAudioMixer*
odAudioMixer_get_singleton();
static void
odAudioMixer_stop_audio_ptr(const odAudio* audio);

static odAudioPlaybackId
odAudioPlaybackId_init(uint8_t channel, uint16_t generation);
static uint8_t
odAudioPlaybackId_get_channel(odAudioPlaybackId playback_id);
static uint16_t
odAudioPlaybackId_get_generation(odAudioPlaybackId playback_id);

struct odAudioMixer {
	bool is_mixer_init;
	uint16_t channel_generation[OD_AUDIO_MIXER_CHANNELS];

	OD_PLATFORM_MODULE odAudioMixer();
	OD_PLATFORM_MODULE ~odAudioMixer();

	odAudioMixer(odAudioMixer&& other) = delete;
	odAudioMixer(odAudioMixer const& other) = delete;
	odAudioMixer& operator=(odAudioMixer&& other) = delete;
	odAudioMixer& operator=(const odAudioMixer& other) = delete;
};

odAudioPlaybackId odAudioPlaybackId_init(uint8_t channel, uint16_t generation) {
	return (
		static_cast<odAudioPlaybackId>(channel)
		+ (static_cast<odAudioPlaybackId>(generation) << OD_AUDIO_PLAYBACK_CHANNEL_BITS));
}
uint8_t odAudioPlaybackId_get_channel(odAudioPlaybackId playback_id) {
	return static_cast<uint8_t>(playback_id & OD_AUDIO_PLAYBACK_CHANNEL_MASK);
}
uint16_t odAudioPlaybackId_get_generation(odAudioPlaybackId playback_id) {
	return static_cast<uint16_t>((playback_id & OD_AUDIO_PLAYBACK_GENERATION_MASK) >> OD_AUDIO_PLAYBACK_CHANNEL_BITS);
}
bool odAudioPlaybackId_check_valid(odAudioPlaybackId playback_id) {
	if (!OD_CHECK(playback_id > 0)
		|| !OD_CHECK(playback_id <= (OD_AUDIO_PLAYBACK_CHANNEL_MASK | OD_AUDIO_PLAYBACK_GENERATION_MASK))
		|| !OD_CHECK(odAudioPlaybackId_get_generation(playback_id) > 0)) {
		return false;
	}

	return true;
}

const odAudioPlaybackSettings* odAudioPlaybackSettings_get_defaults() {
	static const odAudioPlaybackSettings settings{
		/*loop_count*/ 1,
		/*cutoff_time_ms*/ 0,
		/*fade_in_time_ms*/ 0,
		/*volume*/ 1.0f,
		/*is_loop_forever_enabled*/ false,
		/*is_cutoff_time_enabled*/ false,
	};
	return &settings;
}
bool odAudioPlaybackSettings_check_valid(const odAudioPlaybackSettings* settings) {
	if (!OD_CHECK(settings != nullptr)
		|| !OD_CHECK(settings->loop_count >= 0)
		|| !OD_CHECK(settings->cutoff_time_ms >= 0)
		|| !OD_CHECK(settings->fade_in_time_ms >= 0)
		|| !OD_CHECK(odFloat_is_normalized(settings->volume))) {
		return false;
	}

	return true;
}

bool odAudio_init(odAudio* audio) {
	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(odAudioMixer_get_singleton()))) {
		return false;
	}

	const uint8_t wav_one_sample_22050hz_s16[] = {
		0x52, 0x49, 0x46, 0x46, 0x26, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
		0x66, 0x6d, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x22, 0x56, 0x00, 0x00, 0x44, 0xac, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00,
		0x64, 0x61, 0x74, 0x61, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	const int32_t wav_size = sizeof(wav_one_sample_22050hz_s16);

	return odAudio_init_wav(audio, static_cast<const void*>(wav_one_sample_22050hz_s16), wav_size);
}
bool odAudio_init_wav(odAudio* audio, const void* src_wav, int32_t src_wav_size) {
	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(odAudioMixer_get_singleton()))) {
		return false;
	}

	odAudio_destroy(audio);

	SDL_RWops* wav_rw = SDL_RWFromMem(const_cast<void*>(src_wav), static_cast<int>(src_wav_size));
	if (!OD_CHECK(wav_rw != nullptr)) {
		OD_ERROR("SDL_RWFromMem failed, error=%s", SDL_GetError());
		return false;
	}

	audio->audio_native = static_cast<void*>(Mix_LoadWAV_RW(wav_rw, /*freesrc*/ 0));
	if (!OD_CHECK(audio->audio_native != nullptr)) {
		OD_ERROR("Mix_LoadWAV_RW failed, error=%s", Mix_GetError());

		if (!OD_CHECK(SDL_RWclose(wav_rw) == 0)) {
			OD_ERROR("SDL_RWclose failed, error=%s", SDL_GetError());
		}

		return false;
	}
	if (!OD_CHECK(SDL_RWclose(wav_rw) == 0)) {
		OD_ERROR("SDL_RWclose failed, error=%s", SDL_GetError());
		return false;
	}

	return true;
}
bool odAudio_init_wav_file(odAudio* audio, const char* filename) {
	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(odAudioMixer_get_singleton()))
		|| !OD_CHECK(odFile_get_exists(filename))) {
		return false;
	}


	odAllocation allocation{};
	int32_t file_size = 0;
	if (!OD_CHECK(odFile_read_all(filename, "rb", &allocation, &file_size))) {
		return false;
	}

	void* file_data = odAllocation_get(&allocation);
	if (!OD_CHECK(file_data != nullptr)
		|| !OD_CHECK(file_size > 0)) {
		return false;
	}

	if (!OD_CHECK(odAudio_init_wav(audio, file_data, file_size))) {
		return false;
	}

	return true;
}
void odAudio_destroy(odAudio* audio) {
	if (!OD_CHECK(audio != nullptr)) {
		return;
	}

	if (audio->audio_native != nullptr) {
		odAudioMixer_stop_audio_ptr(audio);

		Mix_FreeChunk(static_cast<Mix_Chunk*>(audio->audio_native));
	}
	audio->audio_native = nullptr;
}
void odAudio_swap(odAudio* audio1, odAudio* audio2) {
	if (!OD_CHECK(audio1 != nullptr)
		|| !OD_CHECK(audio2 != nullptr)) {
		return;
	}

	void* audio_native_swap = audio1->audio_native;
	audio1->audio_native = audio2->audio_native;
	audio2->audio_native = audio_native_swap;
}
bool odAudio_check_valid(const odAudio* audio) {
	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(audio->audio_native != nullptr)) {
		return false;
	}

	return true;
}
bool odAudio_set_volume(odAudio* audio, float volume) {
	if (!OD_CHECK(odAudio_check_valid(audio))
		|| !OD_CHECK(odFloat_is_normalized(volume))) {
		return false;
	}

	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return false;
	}

	Mix_VolumeChunk(
		static_cast<Mix_Chunk*>(audio->audio_native),
		static_cast<int>(volume * static_cast<float>(MIX_MAX_VOLUME)));
	return true;
}
odAudioPlaybackId odAudio_play(odAudio* audio, const odAudioPlaybackSettings* opt_settings) {
	if (!OD_CHECK(odAudio_check_valid(audio))
		|| !OD_CHECK((opt_settings == nullptr) || odAudioPlaybackSettings_check_valid(opt_settings))) {
		return 0;
	}

	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return 0;
	}

	odAudioPlaybackSettings settings = *odAudioPlaybackSettings_get_defaults();
	if (opt_settings) {
		settings = *opt_settings;
	}

	int loop_count = static_cast<int>(settings.loop_count);
	if (settings.is_loop_forever_enabled) {
		loop_count = -1;  // -1 = loop forever
	}

	int cutoff_time_ms = static_cast<int>(settings.cutoff_time_ms);
	if (!settings.is_cutoff_time_enabled) {
		cutoff_time_ms = -1;  // -1 = no cutoff
	}

	int fade_in_time_ms = static_cast<int>(settings.fade_in_time_ms);

	Mix_Volume(
		-1,
		static_cast<int>(settings.volume * static_cast<float>(MIX_MAX_VOLUME)));

	int channel = Mix_FadeInChannelTimed(
		/*channel*/ -1,  // -1 = first unused channel
		static_cast<Mix_Chunk*>(audio->audio_native),
		loop_count,
		fade_in_time_ms,
		cutoff_time_ms
	);
	if (!OD_CHECK(channel >= 0)) {
		OD_ERROR("Mix_FadeInChannelTimed() failed, error=%s", Mix_GetError());
		return 0;
	}
	if (!OD_CHECK(channel < OD_AUDIO_MIXER_CHANNELS)) {
		OD_ERROR("Mix_FadeInChannelTimed() returned channel >= OD_AUDIO_MIXER_CHANNELS");
		Mix_HaltChannel(channel);
		return 0;
	}
	if (!OD_CHECK(Mix_Playing(channel) == 1)) {
		OD_ERROR("Mix_FadeInChannelTimed() failed, error=%s", Mix_GetError());
		return 0;
	}

	mixer->channel_generation[channel]++;
	uint16_t generation = mixer->channel_generation[channel];

	return odAudioPlaybackId_init(static_cast<uint8_t>(channel), generation);
}
bool odAudio_stop(odAudioPlaybackId playback_id) {
	if (!OD_CHECK(odAudioPlaybackId_check_valid(playback_id))) {
		return false;
	}

	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return 0;
	}

	uint8_t channel = odAudioPlaybackId_get_channel(playback_id);
	uint16_t generation = odAudioPlaybackId_get_generation(playback_id);

	if (mixer->channel_generation[channel] != generation) {
		return true;
	}

	mixer->channel_generation[channel]++;

	if (!OD_CHECK(Mix_HaltChannel(channel) == 0)) {
		return false;
	}

	return true;
}
bool odAudio_stop_all() {
	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return 0;
	}

	if (!OD_CHECK(Mix_HaltChannel(-1) == 0)) {
		return false;
	}

	return true;
}
bool odAudio_is_playing(odAudioPlaybackId playback_id) {
	if (!OD_CHECK(odAudioPlaybackId_check_valid(playback_id))) {
		return false;
	}

	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return 0;
	}

	uint8_t channel = odAudioPlaybackId_get_channel(playback_id);
	uint16_t generation = odAudioPlaybackId_get_generation(playback_id);

	if (mixer->channel_generation[channel] != generation) {
		return false;
	}

	if (Mix_Playing(channel) == 0) {
		return false;
	}

	return true;
}

odAudio::odAudio()
: audio_native{nullptr} {
}
odAudio::odAudio(odAudio&& other)
: odAudio{} {
	odAudio_swap(this, &other);
}
odAudio& odAudio::operator=(odAudio&& other) {
	odAudio_swap(this, &other);
	return *this;
}
odAudio::~odAudio() {
	odAudio_destroy(this);
}

odAudioMixer* odAudioMixer_get_singleton() {
	static odAudioMixer mixer;

	if (!OD_CHECK(odAudioMixer_check_valid(&mixer))) {
		return nullptr;
	}

	return &mixer;
}
bool odAudioMixer_check_valid(const odAudioMixer* mixer) {
	if (!OD_CHECK(mixer != nullptr)
		|| !OD_CHECK(mixer->is_mixer_init)) {
		return false;
	}

	return true;
}
void odAudioMixer_stop_audio_ptr(const odAudio* audio) {
	if (!OD_CHECK(audio != nullptr)) {
		return;
	}

	odAudioMixer* mixer = odAudioMixer_get_singleton();
	if (!OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return;
	}

	for (int32_t i = 0; i < OD_AUDIO_MIXER_CHANNELS; i++) {
		if (Mix_GetChunk(static_cast<int>(i)) == static_cast<const Mix_Chunk*>(audio->audio_native)) {
			Mix_HaltChannel(i);
		}
	}
}
odAudioMixer::odAudioMixer()
: is_mixer_init{false}, channel_generation{} {
	if (!is_mixer_init) {
		if (!OD_CHECK(odSDLMixer_init_reentrant())) {
			return;
		}
	}
	is_mixer_init = true;

	Mix_AllocateChannels(OD_AUDIO_MIXER_CHANNELS);
}
odAudioMixer::~odAudioMixer() {
	if (is_mixer_init) {
		odSDLMixer_destroy_reentrant();
	}

	is_mixer_init = false;
}
