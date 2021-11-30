#include <od/engine/world.hpp>

#include <od/engine/entity.h>
#include <od/test/test.hpp>

OD_TEST(odWorld, init_destroy) {
	odWorld world;

	OD_ASSERT(odWorld_init(&world));
	odWorld_destroy(&world);
	
	// double init
	OD_ASSERT(odWorld_init(&world));
	OD_ASSERT(odWorld_init(&world));

	// double destroy
	odWorld_destroy(&world);
	odWorld_destroy(&world);
}

OD_TEST(odWorld, set_get) {
	odWorld world{};
	odEntity entity{};
	odEntity compare_entity{};

	OD_ASSERT(odWorld_set(&world, &entity));
	OD_ASSERT(odWorld_get(&world, entity.id, &compare_entity));
	OD_ASSERT(odEntity_equals(&entity, &compare_entity));

	entity.bounds = {0.0f, 0.0f, 16.0f, 16.0f};
	OD_ASSERT(odWorld_set(&world, &entity));
	OD_ASSERT(odWorld_get(&world, entity.id, &compare_entity));
	OD_ASSERT(odEntity_equals(&entity, &compare_entity));

	entity.bounds = {};
	OD_ASSERT(odWorld_set(&world, &entity));
	OD_ASSERT(odWorld_get(&world, entity.id, &compare_entity));
	OD_ASSERT(odEntity_equals(&entity, &compare_entity));

	entity.id = 124;
	entity.tagset = {0x27};
	OD_ASSERT(odWorld_set(&world, &entity));
	OD_ASSERT(odWorld_get(&world, entity.id, &compare_entity));
	OD_ASSERT(odEntity_equals(&entity, &compare_entity));
}

OD_TEST(odWorld, search) {
	odWorld world{};

	const int32_t search_results_count = 4;
	int32_t search_results[search_results_count];
	odWorldSearch search{search_results, 4, {0.0f, 0.0f, 128.0f, 128.0f}, {}, nullptr};
	OD_ASSERT(odWorld_search(&world, &search) == 0);

	odEntity entity{0, {}, {0.0f, 0.0f, 16.0f, 16.0f}};
	OD_ASSERT(odWorld_set(&world, &entity));
	OD_ASSERT(odWorld_search(&world, &search) == 1);

	search.tagset = {1};
	OD_ASSERT(odWorld_search(&world, &search) == 0);
	search.tagset = {};

	search.bounds = {};
	OD_ASSERT(odWorld_search(&world, &search) == 0);
	search.bounds = {0.0f, 0.0f, 128.0f, 128.0f};

	entity.id++;
	entity.bounds = {127.0f, 127.0f, 16.0f, 16.0f};
	OD_ASSERT(odWorld_set(&world, &entity));
	OD_ASSERT(odWorld_search(&world, &search) == 2);

	entity.id++;
	entity.bounds = {128.0f, 128.0f, 16.0f, 16.0f};
	OD_ASSERT(odWorld_set(&world, &entity));
	OD_ASSERT(odWorld_search(&world, &search) == 2);

	search.bounds = {0.0f, 0.0f, 129.0f, 129.0f};
	OD_ASSERT(odWorld_search(&world, &search) == 3);

	// search.bounds = {-64.0f, -64.0f, 128.0f, 128.0f};
}
