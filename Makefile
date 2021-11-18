# Inputs
# ---
# Client code build target.  One of:
# - RELEASE
# - DEBUG
TARGET := DEBUG
# client input arguments 
CLIENT_ARGS :=

# Dependencies
# ---
CMAKE := cmake

# Outputs
# ---
BUILD := build/$(TARGET)
CLIENT := $(BUILD)/od_client

# Commands
# ---
.PHONY: $(CLIENT) run test run_gdb test_gdb profile tidy format clean
.DEFAULT_GOAL := $(CLIENT)

$(CLIENT):
	$(CMAKE) -S . -B $(BUILD) -D CMAKE_BUILD_TYPE=$(TARGET) -G"Ninja" -D BUILD_SHARED_LIBS=1
	$(CMAKE) --build $(BUILD)
run: $(CLIENT)
	$(CLIENT) $(CLIENT_ARGS)
test: $(CLIENT)
	$(CLIENT) --test $(CLIENT_ARGS)
gdb: $(CLIENT)
	gdb -ex 'break main' -ex 'break odLog_on_error' --ex run --args $(CLIENT) $(CLIENT_ARGS)
profile: gmon.out
	gprof -b $(CLIENT)* gmon.out > profile.txt && cat profile.txt
tidy:
	clang-tidy $(shell python -c "import pathlib; print('\n'.join([str(p) for p in pathlib.Path('overworld').rglob('*') if p.suffix in ('.cpp', '.hpp', '.h')]))") -- -Iclient/include -Iclient/src
format:
	clang-format -i -Werror -- $(shell python -c "import pathlib; print('\n'.join([str(p) for p in pathlib.Path('overworld').rglob('*') if p.suffix in ('.cpp', '.hpp', '.h')]))")
clean:
	rm -rf build gmon.out

