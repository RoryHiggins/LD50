# Inputs
# ---
# Client code build target.  One of:
# - RELEASE
# - DEBUG
TARGET := DEBUG
# key for output build
KEY := ANY
# client input arguments 
CLIENT_ARGS :=
CMAKE_ARGS :=

# Dependencies
# ---
CMAKE := cmake

# Outputs
# ---
BUILD_ROOT := build
BUILD := $(BUILD_ROOT)/$(TARGET)_$(KEY)
CLIENT := $(BUILD)/od_client

# Commands
# ---
.PHONY: $(CLIENT) run test run_gdb test_gdb profile tidy format clean
.DEFAULT_GOAL := $(CLIENT)

$(CLIENT):
	$(CMAKE) \-S . -B $(BUILD) -D CMAKE_BUILD_TYPE=$(TARGET) -G"Ninja" $(CMAKE_ARGS)
	$(CMAKE) --build $(BUILD)
run: $(CLIENT)
	$(CLIENT) $(CLIENT_ARGS)
test: $(CLIENT)
	$(CLIENT) --no-client --test $(CLIENT_ARGS)
gdb: $(CLIENT)
	gdb --ex 'break main' --ex "break odDebug_error" --ex "run" --args $(CLIENT) --test $(CLIENT_ARGS)
profile: gmon.out
	gprof -b $(CLIENT)* gmon.out > profile.txt && cat profile.txt
tidy:
	clang-tidy $(shell python -c "import pathlib; print('\n'.join([str(p) for p in pathlib.Path('client').rglob('*') if p.suffix in ('.cpp', '.hpp', '.h')]))") -- -Iclient/include -Iclient/src
format:
	clang-format -i -Werror -- $(shell python -c "import pathlib; print('\n'.join([str(p) for p in pathlib.Path('client').rglob('*') if p.suffix in ('.cpp', '.hpp', '.h')]))")
coverage:
	gcov -mt $(shell python -c "import pathlib; print('\n'.join([p.as_posix() for p in pathlib.Path(r'$(BUILD)').rglob('*') if p.suffix == '.gcda']))") > coverage.txt
clean:
	rm -rf $(BUILD_ROOT) gmon.out profile.txt

