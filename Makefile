# Inputs
# ---
# Client code build target.  One of:
# - RELEASE
# - PROFILE
# - DEBUG
TARGET := DEBUG
# key for output build
KEY := ANY
# client input arguments
CLIENT_SRC := ./client
CLIENT_ARGS :=
CMAKE_GENERATE_ARGS :=
CMAKE_BUILD_ARGS :=
ENGINE_TEST_ENTRYPOINT :=

# Dependencies
# ---
CMAKE := cmake
LUACHECK := luacheck
PYTHON := python

# Outputs
# ---
BUILD_ROOT := build
BUILD := $(BUILD_ROOT)/$(TARGET)_$(KEY)
CLIENT := $(BUILD)/od_client
LUA_CLIENT := examples/minimal/main.lua

# Commands
# ---
.PHONY: $(CLIENT) run test run_gdb test_gdb profile tidy format clean
.DEFAULT_GOAL := $(CLIENT)

$(CLIENT):
	$(CMAKE) \-S $(CLIENT_SRC) -B $(BUILD) -D CMAKE_BUILD_TYPE=$(TARGET) -G"Ninja" $(CMAKE_GENERATE_ARGS)
	$(CMAKE) --build $(BUILD) -- $(CMAKE_BUILD_ARGS)
run: $(CLIENT)
	$(CLIENT) --lua-client "$(LUA_CLIENT)" $(CLIENT_ARGS)
engine_test: $(CLIENT)
	$(PYTHON) scripts/generate_engine_all.py --entrypoint="$(ENGINE_TEST_ENTRYPOINT)" > examples/engine_test/all.lua
	$(LUACHECK) engine examples
	$(CLIENT) --test --no-slow-test --lua-client examples/engine_test/all.lua $(CLIENT_ARGS)
engine_test_minimal:
	$(PYTHON) scripts/generate_engine_all.py --entrypoint="$(ENGINE_TEST_ENTRYPOINT)" > examples/engine_test/all.lua
	$(CLIENT) --lua-client examples/engine_test/all.lua $(CLIENT_ARGS)
test: $(CLIENT)
	$(CLIENT) --no-lua-client --test $(CLIENT_ARGS)
gdb: $(CLIENT)
	gdb --ex 'break main' --ex "break odDebug_error" --ex "run" --args $(CLIENT) --test --lua-client "$(LUA_CLIENT)" $(CLIENT_ARGS)
profile: gmon.out
	gprof -b $(CLIENT)* gmon.out > profile.txt && cat profile.txt
tidy:
	clang-tidy $(shell $(PYTHON) -c "import pathlib; print('\n'.join([str(p) for p in pathlib.Path('client').rglob('*') if p.suffix in ('.cpp', '.hpp', '.h')]))") -- -Iclient/include -Iclient/src
format:
	clang-format -i -Werror -- $(shell $(PYTHON) -c "import pathlib; print('\n'.join([str(p) for p in pathlib.Path('client').rglob('*') if p.suffix in ('.cpp', '.hpp', '.h')]))")
coverage:
	gcov -mt $(shell $(PYTHON) -c "import pathlib; print('\n'.join([p.as_posix() for p in pathlib.Path(r'$(BUILD)').rglob('*') if p.suffix == '.gcda']))") > coverage.txt
clean:
	rm -rf $(BUILD_ROOT) gmon.out profile.txt

