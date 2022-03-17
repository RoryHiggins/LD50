#include <od/platform/sdl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <od/core/debug.h>

static int32_t odSDL_init_counter = 0;
static int32_t odSDLMixer_init_counter = 0;

bool odSDL_init_reentrant() {
	OD_DEBUG("odSDL_init_counter=%d", odSDL_init_counter);

	if (odSDL_init_counter == 0) {
		OD_DEBUG("odSDL init");

		const Uint32 sdl_flags = (
			SDL_INIT_EVENTS
			| SDL_INIT_TIMER
			| SDL_INIT_VIDEO
			| SDL_INIT_AUDIO
		);
		int init_result = SDL_Init(sdl_flags);
		if (!OD_CHECK(init_result == 0)) {
			OD_ERROR("SDL_Init failed, init_result=%d", init_result);
			return false;
		}
	}

	odSDL_init_counter++;
	return true;
}
void odSDL_destroy_reentrant() {
	OD_DEBUG("odSDL_init_counter=%d", odSDL_init_counter);

	if (odSDL_init_counter <= 0) {
		OD_WARN("odSDL_destroy_reentrant with no matching odSDL_init_reentrant");
		return;
	}

	odSDL_init_counter--;

	if (odSDL_init_counter == 0) {
		OD_DEBUG("odSDL destroy");
		SDL_Quit();
	}
}

bool odSDLMixer_init_reentrant() {
	OD_DEBUG("odSDLMixer_init_counter=%d", odSDLMixer_init_counter);

	if (odSDLMixer_init_counter == 0) {
		OD_DEBUG("odSDLMixer init");

		if (!OD_CHECK(odSDL_init_reentrant())) {
			return false;
		}

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
void odSDLMixer_destroy_reentrant() {
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

		odSDL_destroy_reentrant();
	}
}
