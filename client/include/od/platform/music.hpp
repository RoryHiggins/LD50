#pragma once

#include <od/platform/music.h>

struct odMusic {
	void* music_native;

	OD_PLATFORM_MODULE odMusic();
	OD_PLATFORM_MODULE odMusic(odMusic&& other);
	OD_PLATFORM_MODULE odMusic& operator=(odMusic&& other);
	OD_PLATFORM_MODULE ~odMusic();

	odMusic(odMusic const& other) = delete;
	odMusic& operator=(const odMusic& other) = delete;
};
