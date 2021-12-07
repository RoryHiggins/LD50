#pragma once

#include <od/engine/entity.h>

#include <od/core/fast_array.hpp>
#include <od/core/array.hpp>

OD_ENGINE_MODULE extern template struct odFastArrayT<odEntityCollider>;
OD_ENGINE_MODULE extern template struct odFastArrayT<odEntity>;
