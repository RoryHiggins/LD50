#include <od/platform/audio.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <od/core/debug.h>
#include <od/core/box.hpp>
#include <od/core/array.hpp>

OD_NO_DISCARD static bool
odSDLMixer_init_reentrant();
static void
odSDLMixer_destroy_reentrant();

static int32_t odSDLMixer_init_counter = 0;

bool odSDLMixer_init_reentrant() {
	OD_DEBUG("odSDLMixer_init_counter=%d", odSDLMixer_init_counter);

	if (odSDLMixer_init_counter == 0) {
		OD_DEBUG("odSDLMixer init");
		int mixer_flags = 0;
		int init_result = Mix_Init(mixer_flags);
		if (!OD_CHECK(init_result == mixer_flags)) {
			OD_ERROR("Mix_Init failed, init_result=%d, expected_result=%d, err=%s", init_result, mixer_flags, Mix_GetError());
			return false;
		}

		int channels = 2;  // stereo
		int chunk_size = 1024; // smallest commonly used value to avoid skipping while staying low latency
		if (!OD_CHECK(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, channels, chunk_size) == 0)) {
			OD_ERROR("Mix_OpenAudio failed, err=%s", Mix_GetError());
			return false;
		}
	}

	odSDLMixer_init_counter++;
	return true;
}
static void odSDLMixer_destroy_reentrant() {
	OD_DEBUG("odSDLMixer_init_counter=%d", odSDLMixer_init_counter);

	if (odSDLMixer_init_counter <= 0) {
		OD_WARN("odSDLMixer_destroy_reentrant with no matching odSDLMixer_init_reentrant");
		return;
	}

	odSDLMixer_init_counter--;

	if (odSDLMixer_init_counter == 0) {
		OD_DEBUG("odSDLMixer destroy");
		Mix_CloseAudio();

		Mix_Quit();
	}
}

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
