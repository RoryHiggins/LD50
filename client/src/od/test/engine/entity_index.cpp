#include <od/engine/entity_index.hpp>

#include <od/core/debug.h>
#include <od/core/bounds.h>
#include <od/platform/timer.h>
#include <od/engine/entity.h>
#include <od/test/test.hpp>

OD_TEST(odTest_odEntityIndex_init_destroy) {
	odEntityIndex entity_index;

	odEntityIndex_init(&entity_index);
	odEntityIndex_destroy(&entity_index);

	// double init
	odEntityIndex_init(&entity_index);
	odEntityIndex_init(&entity_index);

	// double destroy
	odEntityIndex_destroy(&entity_index);
	odEntityIndex_destroy(&entity_index);
}
OD_TEST(odTest_odEntityIndex_set_get) {
	odEntityIndex entity_index{};
	odEntity entity{};

	odEntityIndex_set(&entity_index, &entity);
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_get_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));

	entity.collider.bounds = odBounds{0.0f, 0.0f, 16.0f, 16.0f};
	odEntityIndex_set(&entity_index, &entity);
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_get_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));

	entity.collider.bounds = odBounds{};
	odEntityIndex_set(&entity_index, &entity);
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_get_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));

	entity.collider.id = 124;
	entity.collider.tagset = odTagset{0x27};
	odEntityIndex_set(&entity_index, &entity);
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_get_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));
}
OD_TEST(odTest_odEntityIndex_search) {
	odEntityIndex entity_index{};

	// OD_INFO("%s", odEntityIndex_get_debug_string(&entity_index));

	const int32_t search_results_count = 4;
	int32_t search_results[search_results_count];
	odEntitySearch search{search_results, search_results_count, odBounds{0.0f, 0.0f, 128.0f, 128.0f}, odTagset{}, nullptr};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	odEntity entity{};
	entity.collider.bounds = odBounds{0.0f, 0.0f, 16.0f, 16.0f};
	odEntityIndex_set(&entity_index, &entity);
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 1);

	search.tagset = odTagset{1};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);
	search.tagset = odTagset{};

	search.bounds = odBounds{};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);
	search.bounds = odBounds{0, 0, 128, 128};

	entity.collider.id++;
	entity.collider.bounds = odBounds{127.0f, 127.0f, 143.0f, 143.0f};
	odEntityIndex_set(&entity_index, &entity);
	OD_ASSERT(entity_index.entities.get_count() >= 2);
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 2);

	entity.collider.id++;
	entity.collider.bounds = odBounds{128.0f, 128.0f, 144.0f, 144.0f};
	odEntityIndex_set(&entity_index, &entity);
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 2);

	search.bounds = odBounds{0.0f, 0.0f, 129.0f, 129.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 3);

	search.bounds = odBounds{-1.0f, -1.0f, 129.0f, 129.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 3);

	search.bounds = odBounds{-64.0f, -64.0f, 129.0f, 129.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 3);

	search.bounds = odBounds{-64.0f, -64.0f, 128.0f, 128.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 2);

	search.bounds = odBounds{-64.0f, -64.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 1);

	search.bounds = odBounds{-64.0f, -64.0f, 0.0f, 0.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = odBounds{-64.0f, -64.0f, -63.0f, -63.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	entity.collider.id++;
	entity.collider.bounds = odBounds{-64.0f, -64.0f, -63.0f, -63.0f};
	odEntityIndex_set(&entity_index, &entity);

	search.bounds = odBounds{-64.0f, -64.0f, -63.0f, -63.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 1);

	search.bounds = odBounds{-63.0f, -63.0f, -62.0f, -62.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = odBounds{-65.0f, -65.0f, -65.0f, -65.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = odBounds{-64.0f, -65.0f, -63.0f, -64.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = odBounds{-65.0f, -64.0f, -64.0f, -63.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);
}
OD_TEST_FILTERED(odTest_odEntityIndex_search_performance, OD_TEST_FILTER_SLOW) {
	const int32_t tile_width = 8;
	const float tile_width_f = static_cast<float>(tile_width);
	const int32_t grid_width_bits = (
		OD_ENTITY_CHUNK_OPTIMUM_WORLD_WIDTH_BITS - OD_ENTITY_CHUNK_OPTIMUM_CHUNK_WIDTH_BITS);
	const int32_t grid_tile_width = (1 << grid_width_bits);
	const int32_t grid_width = grid_tile_width * tile_width;
	const float grid_width_f = static_cast<float>(grid_width);
	const int32_t grid_tile_area = grid_tile_width * grid_tile_width;

	const int32_t entity_density = 1;
	const int32_t entities_count = grid_tile_area * entity_density;
	const int32_t dynamic_entities_count = 32;
	const int32_t large_searches_count = 4;
	const int32_t small_search_results_count = 4;
	const int32_t large_search_results_count = entities_count;
	const int32_t updates_count = dynamic_entities_count;
	const int32_t small_searches_count = dynamic_entities_count * 8;

	const int32_t frame_rate = 60;
	const int32_t seconds_to_test = 5;
	const int32_t frames_to_test = frame_rate * seconds_to_test;

	odEntityIndex entity_index{};
	for (int32_t i = 0; i < entities_count; i++) {
		float x = static_cast<float>(tile_width % grid_tile_width);
		float y = static_cast<float>((tile_width * (i / grid_tile_width)) % grid_tile_width);

		odEntity entity{};
		entity.collider.id = i;
		entity.collider.bounds = odBounds{x, y, x + tile_width_f, y + tile_width_f};
		odEntityIndex_set(&entity_index, &entity);
	}

	odTimer timer;
	odTimer_start(&timer);

	for (int32_t i = 0; i < frames_to_test; i++) {
		for (int32_t j = 0; j < updates_count; j++) {
			int32_t entity_id = ((updates_count * i) + j) % entities_count;

			const odEntity* src_entity = odEntityIndex_get(&entity_index, entity_id);
			OD_ASSERT(src_entity != nullptr);
			odEntity entity = *src_entity;

			float offset = 1.0f;
			if (static_cast<int32_t>(entity.collider.bounds.x1) % tile_width != 0) {
				offset = -1.0f;
			}

			entity.collider.bounds.x1 += offset;
			entity.collider.bounds.x2 += offset;
			entity.collider.bounds.y1 += offset;
			entity.collider.bounds.y2 += offset;

			odEntityIndex_set(&entity_index, &entity);
		}

		int32_t cumulative = 0;
		for (int32_t j = 0; j < small_searches_count; j++) {
			float x = static_cast<float>(tile_width % grid_tile_width);
			float y = static_cast<float>((tile_width * (i / grid_tile_width)) % grid_tile_width);

			static int32_t search_results[small_search_results_count];
			odEntitySearch search{
				search_results,
				small_search_results_count,
				odBounds{x, y, x + tile_width_f + 1.0f, y + tile_width_f + 1.0f},
				odTagset{},
				nullptr};
			cumulative += odEntityIndex_search(&entity_index, &search);
		}
		OD_ASSERT(cumulative > 0);

		cumulative = 0;
		for (int32_t j = 0; j < large_searches_count; j++) {
			static int32_t search_results[large_search_results_count];
			odEntitySearch search{
				search_results,
				large_search_results_count,
				odBounds{0.0f, 0.0f, grid_width_f / 4.0f, grid_width_f / 4.0f},
				odTagset{},
				nullptr};
			cumulative += odEntityIndex_search(&entity_index, &search);
		}
		OD_ASSERT(cumulative > 0);
	}

	OD_INFO(
		"frames_to_test=%d,\nupdates_total=%d,\nsmall_searches_total=%d,\nlarge_searches_total=%d,\n"
		"entities_count=%d,elapsed_sec=%g",
		frames_to_test,
		updates_count * frames_to_test,
		small_searches_count * frames_to_test,
		large_searches_count * frames_to_test,
		entities_count,
		static_cast<double>(odTimer_get_elapsed_seconds(&timer))
	);

	OD_TIMER_WARN_IF_EXCEEDED(&timer, seconds_to_test);
}

OD_TEST_SUITE(
	odTestSuite_odEntityIndex,
	odTest_odEntityIndex_init_destroy,
	odTest_odEntityIndex_set_get,
	odTest_odEntityIndex_search,
	odTest_odEntityIndex_search_performance,
)
