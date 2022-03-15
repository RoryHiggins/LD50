#include <od/core/box.hpp>

#include <cstring>

#include <od/test/test.hpp>

struct odBoxTestingContainer;

struct odBoxTestingContainer {
	const odBoxTestingContainer* original_self;
	int32_t move_assign_count;
	int32_t destruct_count;

	odBoxTestingContainer();
	odBoxTestingContainer(odBoxTestingContainer&&);
	odBoxTestingContainer& operator=(odBoxTestingContainer&& other);
	~odBoxTestingContainer();

	odBoxTestingContainer(const odBoxTestingContainer&) = delete;
	odBoxTestingContainer& operator=(const odBoxTestingContainer&) = delete;
};

odBoxTestingContainer::odBoxTestingContainer()
	: original_self{this}, move_assign_count{0}, destruct_count{0} {
}
odBoxTestingContainer::odBoxTestingContainer(odBoxTestingContainer&& other) {
	original_self = other.original_self;
	move_assign_count = other.move_assign_count + 1;
	destruct_count = other.destruct_count;
}
odBoxTestingContainer& odBoxTestingContainer::operator=(odBoxTestingContainer&& other) {
	original_self = other.original_self;
	move_assign_count = other.move_assign_count + 1;
	destruct_count = other.destruct_count;

	return *this;
}
odBoxTestingContainer::~odBoxTestingContainer() {
	destruct_count++;
}

OD_TEST(odTest_odBox_init_ensure_destroy) {
	odBoxT<odBoxTestingContainer> box;
	OD_ASSERT(odBox_check_valid(&box));

	OD_ASSERT(odBox_init(&box, odType_get<odBoxTestingContainer>()));
	OD_ASSERT(odBox_check_valid(&box));

	OD_ASSERT(box.ensure() != nullptr);
	OD_ASSERT(odBox_check_valid(&box));
	OD_ASSERT(box.get() != nullptr);

	odBox_destroy(&box);
	OD_ASSERT(box.get() == nullptr);

	// double destroy
	odBox_destroy(&box);

	// reinit
	OD_ASSERT(odBox_init(&box, odType_get<odBoxTestingContainer>()));
	OD_ASSERT(odBox_check_valid(&box));

	OD_ASSERT(box.ensure());
	OD_ASSERT(odBox_check_valid(&box));
	OD_ASSERT(box.get() != nullptr);
}

OD_TEST_SUITE(
	odTestSuite_odBox,
	odTest_odBox_init_ensure_destroy,
)
