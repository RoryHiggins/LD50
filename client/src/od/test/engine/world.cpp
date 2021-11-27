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

// OD_TEST(odWorld, set) {
// 	odWorld world;
// 	odEntity entity;

// 	OD_ASSERT(odWorld_set(&world, &entity));
// }
