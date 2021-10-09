# Inputs
# ---
# Client code build target.  One of:
# - RELEASE
# - DEBUG
TARGET := DEBUG

# Dependencies
# ---
CMAKE := cmake

# Outputs
# ---
BUILD := build/$(TARGET)
CLIENT := $(BUILD)/od_client
TEST := $(BUILD)/od_test

# Commands
# ---
.PHONY: $(CLIENT) run test run_gdb test_gdb profile tidy format clean
.DEFAULT_GOAL := $(CLIENT)

$(CLIENT):
	$(CMAKE) -S . -B $(BUILD) -D CMAKE_BUILD_TYPE=$(TARGET) -G"Ninja" -D BUILD_SHARED_LIBS=1 -D OD_BUILD_DEBUG_LOG=1
	$(CMAKE) --build $(BUILD)
$(TEST): $(CLIENT)
	$(CMAKE) --build $(BUILD) --target od_test
run: $(CLIENT)
	$(CLIENT)
test: $(TEST)
	$(TEST)
run_gdb: $(CLIENT)
	gdb -ex 'break main' --ex run --args $(CLIENT) --trace
test_gdb: $(CLIENT)
	gdb -ex 'break main' --ex run --args $(TEST)
profile: gmon.out
	gprof -b $(CLIENT)* gmon.out > profile.txt && cat profile.txt
tidy:
	find ./client -name '*.c' -or -name '*.h' | xargs -I TIDY_INPUT clang-tidy TIDY_INPUT -- -Iclient/include -Iclient/src
format:
	find ./client -name '*.c' -or -name '*.h' | xargs clang-format -i -Werror --
clean:
	rm -rf build

