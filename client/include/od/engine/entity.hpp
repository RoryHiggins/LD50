#pragma once

#include <od/engine/entity.h>

#include <od/core/trivial_array.hpp>
#include <od/core/array.hpp>

OD_ENGINE_MODULE extern template struct odTrivialArrayT<odEntityCollider>;
OD_ENGINE_MODULE extern template struct odTrivialArrayT<odEntity>;
