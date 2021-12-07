#include <od/engine/entity_index.hpp>

#include <od/platform/timer.h>
#include <od/engine/entity.h>
#include <od/test/test.hpp>

OD_TEST(odTest_odEntityIndex_init_destroy) {
	odEntityIndex entity_index;

	OD_ASSERT(odEntityIndex_init(&entity_index));
	odEntityIndex_destroy(&entity_index);
	
	// double init
	OD_ASSERT(odEntityIndex_init(&entity_index));
	OD_ASSERT(odEntityIndex_init(&entity_index));

	// double destroy
	odEntityIndex_destroy(&entity_index);
	odEntityIndex_destroy(&entity_index);
}
OD_TEST(odTest_odEntityIndex_set_get) {
	odEntityIndex entity_index{};
	odEntity entity{};

	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));

	entity.collider.bounds = {0.0f, 0.0f, 16.0f, 16.0f};
	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));

	entity.collider.bounds = {};
	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));

	entity.collider.id = 124;
	entity.collider.tagset = {0x27};
	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	OD_ASSERT(odEntityIndex_get(&entity_index, entity.collider.id) != nullptr);
	OD_ASSERT(odEntityCollider_equals(&entity.collider,
									  &odEntityIndex_get(&entity_index, entity.collider.id)->collider));
}
OD_TEST(odTest_odEntityIndex_search) {
	odEntityIndex entity_index{};

	// OD_INFO("%s", odEntityIndex_get_debug_string(&entity_index));

	const int32_t search_results_count = 4;
	int32_t search_results[search_results_count];
	odEntitySearch search{search_results, search_results_count, {0.0f, 0.0f, 128.0f, 128.0f}, {}};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	odEntity entity{};
	entity.collider = odEntityCollider{0, {}, {0.0f, 0.0f, 16.0f, 16.0f}};
	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 1);

	search.tagset = {1};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);
	search.tagset = {};

	search.bounds = {};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);
	search.bounds = {0.0f, 0.0f, 128.0f, 128.0f};

	entity.collider.id++;
	entity.collider.bounds = {127.0f, 127.0f, 16.0f, 16.0f};
	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	OD_ASSERT(entity_index.entities.count >= 2);
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 2);

	entity.collider.id++;
	entity.collider.bounds = {128.0f, 128.0f, 16.0f, 16.0f};
	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 2);

	search.bounds = {0.0f, 0.0f, 129.0f, 129.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 3);

	search.bounds = {-1.0f, -1.0f, 130.0f, 130.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 3);

	search.bounds = {-64.0f, -64.0f, 193.0f, 193.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 3);

	search.bounds = {-64.0f, -64.0f, 192.0f, 192.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 2);

	search.bounds = {-64.0f, -64.0f, 65.0f, 65.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 1);

	search.bounds = {-64.0f, -64.0f, 64.0f, 64.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = {-64.0f, -64.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	entity.collider.id++;
	entity.collider.bounds = {-64.0f, -64.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_set(&entity_index, &entity));

	search.bounds = {-64.0f, -64.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 1);

	search.bounds = {-63.0f, -63.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = {-65.0f, -65.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = {-64.0f, -65.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);

	search.bounds = {-65.0f, -64.0f, 1.0f, 1.0f};
	OD_ASSERT(odEntityIndex_search(&entity_index, &search) == 0);
}
OD_TEST_FILTERED(odTest_odEntityIndex_search_performance, OD_TEST_FILTER_SLOW) {
	const int32_t grid_width_bits = (
		OD_ENTITY_CHUNK_OPTIMUM_WORLD_WIDTH_BITS - OD_ENTITY_CHUNK_OPTIMUM_CHUNK_WIDTH_BITS);
	const int32_t grid_width = (1 << grid_width_bits);
	const float grid_width_f = float(grid_width);
	const int32_t grid_area = (grid_width * grid_width);

	const int32_t entities_count = grid_area / 8;
	const int32_t updates_count = entities_count / 16;
	const int32_t small_searches_count = (entities_count * 2);
	const int32_t large_searches_count = 4;
	const int32_t search_results_count = 1;

	const int32_t frame_rate = 60;
	const int32_t seconds_to_test = 5;
	const int32_t frames_to_test = frame_rate * seconds_to_test;
	const float max_duration_seconds = float(seconds_to_test);

	odEntityIndex entity_index{};
	for (int32_t i = 0; i < entities_count; i++) {
		odEntity entity{};
		entity.collider.id = i;
		entity.collider.bounds = odBounds{0.0f * float(i % 64), 0.0f * float(i / 64), 8.0f, 8.0f};
		OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
	}

	odTimer timer;
	odTimer_start(&timer);

	for (int32_t i = 0; i < frames_to_test; i++) {
		for (int32_t j = 0; j < updates_count; j++) {
			odEntity entity{};
			entity.collider.id = j;
			entity.collider.bounds = odBounds{0.0f * float((i + j) % 64), 0.0f * float((i + j) / 64), 8.0f, 8.0f};
			OD_ASSERT(odEntityIndex_set(&entity_index, &entity));
		}

		int32_t cumulative = 0;
		for (int32_t j = 0; j < small_searches_count; j++) {
			int32_t search_results[search_results_count];
			odEntitySearch search{
				search_results,
				search_results_count,
				{8.0f * float(j / 64), 8.0f * float(j % 64), 8.0f, 8.0f},
				{}};
			cumulative += odEntityIndex_search(&entity_index, &search);
		}
		OD_ASSERT(cumulative > 0);

		cumulative = 0;
		for (int32_t j = 0; j < large_searches_count; j++) {
			int32_t search_results[search_results_count];
			odEntitySearch search{
				search_results,
				search_results_count,
				{-grid_width_f, -grid_width_f, (2.0f * grid_width_f), (2.0f * grid_width_f)},
				{}};
			cumulative += odEntityIndex_search(&entity_index, &search);
		}
		OD_ASSERT(cumulative > 0);
	}

	OD_TIMER_WARN_IF_EXCEEDED(&timer, max_duration_seconds);
}

OD_TEST_SUITE(
	odTestSuite_odEntityIndex,
	odTest_odEntityIndex_init_destroy,
	odTest_odEntityIndex_set_get,
	odTest_odEntityIndex_search,
	odTest_odEntityIndex_search_performance,
)