#pragma once

#include <od/engine/world.hpp>

// static uint8_t odWorld_get_chunk_coord(float coord) {
// 	const int32_t coord_bitmask = (1 << OD_WORLD_CHUNK_COORD_MASK_BITS) - 1;
// 	return (static_cast<int32_t>(coord) >> OD_WORLD_CHUNK_COORD_DISCARD_BITS) & coord_bitmask;
// }
// static int32_t odWorld_get_chunk_id(float x, float y) {
// 	return (
// 		static_cast<int32_t>(odWorld_get_chunk_coord(x))
// 		+ static_cast<int32_t>((odWorld_get_chunk_coord(y) << OD_WORLD_CHUNK_COORD_MASK_BITS)));
// }

odWorld::odWorld() = default;
odWorld::odWorld(odWorld&& other) = default;
odWorld& odWorld::operator=(odWorld&& other) = default;
odWorld::~odWorld() = default;
