#include <od/platform/audio.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <od/core/debug.h>
#include <od/platform/sdl.h>
#include <od/platform/file.h>

bool odAudioMixer_init(odAudioMixer* mixer) {
	if (!OD_CHECK(mixer != nullptr)) {
		return false;
	}

	odAudioMixer_destroy(mixer);

	if (!mixer->is_mixer_init) {
		if (!OD_CHECK(odSDLMixer_init_reentrant())) {
			return false;
		}
	}
	mixer->is_mixer_init = true;

	return true;
}
void odAudioMixer_destroy(odAudioMixer* mixer) {
	if (!OD_CHECK(mixer != nullptr)) {
		return;
	}

	for (odAudioMixerResource* resource: mixer->resources) {
		resource->mixer = nullptr;
	}
	OD_DISCARD(OD_CHECK(mixer->resources.set_count(0)));

	if (mixer->is_mixer_init) {
		odSDLMixer_destroy_reentrant();
	}

	mixer->is_mixer_init = false;
}
void odAudioMixer_swap(odAudioMixer* mixer1, odAudioMixer* mixer2) {
	if (!OD_CHECK(mixer1 != nullptr)
		|| !OD_CHECK(mixer2 != nullptr)) {
		return;
	}

	bool is_mixer_init_swap = mixer1->is_mixer_init;
	mixer1->is_mixer_init = mixer2->is_mixer_init;
	mixer2->is_mixer_init = is_mixer_init_swap;

	odTrivialArray_swap(&mixer1->resources, &mixer2->resources);
}
bool odAudioMixer_check_valid(const odAudioMixer* mixer) {
	if (!OD_CHECK(mixer != nullptr)
		|| !OD_CHECK(mixer->is_mixer_init)) {
		return false;
	}

	return true;
}
odAudioMixer::odAudioMixer()
: is_mixer_init{false} {
	OD_DISCARD(OD_CHECK(odAudioMixer_init(this)));
}
odAudioMixer::odAudioMixer(odAudioMixer&& other)
: odAudioMixer{} {
	odAudioMixer_swap(this, &other);odAudioMixer_swap(this, &other);
}
odAudioMixer& odAudioMixer::operator=(odAudioMixer&& other) {
	odAudioMixer_swap(this, &other);
	return *this;
}
odAudioMixer::~odAudioMixer() {
	odAudioMixer_destroy(this);
}

bool odAudioMixerResource_init(odAudioMixerResource* resource, odAudioMixer* opt_mixer) {
	if (!OD_CHECK(resource != nullptr)
		|| !OD_CHECK((opt_mixer == nullptr) || odAudioMixer_check_valid(opt_mixer))) {
		return false;
	}

	odAudioMixerResource_destroy(resource);

	if (opt_mixer == nullptr) {
		return true;
	}
	
	if (!OD_CHECK(opt_mixer->resources.push(resource))) {
		return false;
	}

	resource->mixer = opt_mixer;

	return true;
}
void odAudioMixerResource_destroy(odAudioMixerResource* resource) {
	if (!OD_CHECK(resource != nullptr)) {
		return;
	}

	if (resource->mixer == nullptr) {
		return;
	}
	odAudioMixer* mixer = resource->mixer;

	resource->mixer = nullptr;

	odAudioMixerResource** resources = mixer->resources.begin();
	int32_t resources_count = mixer->resources.get_count();
	for (int32_t i = 0; i < resources_count; i++) {
		if (resources[i] == resource) {
			OD_DISCARD(OD_CHECK(mixer->resources.swap_pop(i)));
			break;
		}
	}
}
odAudioMixerResource::odAudioMixerResource()
: mixer{nullptr} {
}
odAudioMixerResource::odAudioMixerResource(odAudioMixerResource&& other)
: odAudioMixerResource{} {
	OD_DISCARD(OD_CHECK(odAudioMixerResource_init(this, other.mixer)));
	odAudioMixerResource_destroy(&other);
}
odAudioMixerResource& odAudioMixerResource::operator=(odAudioMixerResource&& other) {
	OD_DISCARD(OD_CHECK(odAudioMixerResource_init(this, other.mixer)));
	odAudioMixerResource_destroy(&other);
	return *this;
}
odAudioMixerResource::~odAudioMixerResource() {
	odAudioMixerResource_destroy(this);
}

bool odAudio_init(odAudio* audio, odAudioMixer* mixer) {
	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return false;
	}

	const uint8_t wav_empty[] = {
		0x52, 0x49, 0x46, 0x46, 0x24, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
		0x66, 0x6d, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
		0x22, 0x56, 0x00, 0x00, 0x44, 0xac, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00,
		0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00
	};
	const int32_t wav_size = sizeof(wav_empty);

	return odAudio_init_wav(audio, mixer, static_cast<const void*>(wav_empty), wav_size);
}
bool odAudio_init_wav(odAudio* audio, odAudioMixer* mixer, const void* src_wav, int32_t src_wav_size) {
	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(mixer))) {
		return false;
	}

	odAudio_destroy(audio);

	if (!OD_CHECK(odAudioMixerResource_init(audio, mixer))) {
		return false;
	}

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
bool odAudio_init_wav_file(odAudio* audio, odAudioMixer* mixer, const char* filename) {
	if (!OD_CHECK(audio != nullptr)
		|| !OD_CHECK(odAudioMixer_check_valid(mixer))
		|| !OD_CHECK(odFile_get_exists(filename))) {
		return false;
	}


	struct odAllocation allocation{};
	int32_t file_size = 0;
	if (!OD_CHECK(odFile_read_all(filename, "rb", &allocation, &file_size))) {
		return false;
	}

	void* file_data = odAllocation_get(&allocation);
	if (!OD_CHECK(file_data != nullptr)
		|| !OD_CHECK(file_size > 0)) {
		return false;
	}

	if (!OD_CHECK(odAudio_init_wav(audio, mixer, file_data, file_size))) {
		return false;
	}

	return true;
}
void odAudio_destroy(odAudio* audio) {
	if (!OD_CHECK(audio != nullptr)) {
		return;
	}

	if (audio->audio_native != nullptr) {
		Mix_FreeChunk(static_cast<Mix_Chunk*>(audio->audio_native));
	}
	audio->audio_native = nullptr;

	odAudioMixerResource_destroy(audio);
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
		|| !OD_CHECK(odAudioMixer_check_valid(audio->mixer)
		|| !OD_CHECK(audio->audio_native != nullptr))) {
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
