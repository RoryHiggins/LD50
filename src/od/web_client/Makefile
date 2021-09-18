PROJECT_DIR := ../../..

BUILD_DIR := $(PROJECT_DIR)/build/EMSCRIPTEN
BUILD_FILE := $(BUILD_DIR)/index.html

SRC_DIR := $(PROJECT_DIR)/src
SRC_FILES := $(SRC_DIR)/od/client/main.cpp $(SRC_DIR)/od/core/allocation.cpp $(SRC_DIR)/od/core/array.cpp $(SRC_DIR)/od/core/box.cpp $(SRC_DIR)/od/core/debug.cpp $(SRC_DIR)/od/core/string.cpp $(SRC_DIR)/od/core/type.cpp $(SRC_DIR)/od/platform/window.cpp

SHELL_FILE := shell_minimal.html

$(BUILD_FILE): $(SRC_FILES) $(SHELL_FILE) Makefile
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	em++ -o $(BUILD_FILE) $(SRC_FILES) --shell-file $(SHELL_FILE) -D OD_BUILD_RENDERER=2 -D OD_BUILD_EMSCRIPTEN -I$(PROJECT_DIR)/include -I$(PROJECT_DIR)/src -s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1 -s NO_FILESYSTEM=1 -Wall

host:
	python3 -m http.server -d $(BUILD_DIR)

.PHONY: host
