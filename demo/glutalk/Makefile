GLU_ROOT ?= $(abspath ../..)
BUILD_DIR := $(abspath build)

GLUC ?= $(GLU_ROOT)/build/tools/gluc/gluc
CLANG ?= clang++
RUSTC ?= rustc

CPP_SRC := src/cpp/presenter.cpp
RUST_SRC := src/rust/lib.rs
GLU_SRC := src/glu/main.glu

GLU_STDLIB := $(wildcard $(GLU_ROOT)/build/tools/lib/glu/*/*.o)

CPP_BC := $(BUILD_DIR)/glutalk_cpp.bc
CPP_OBJ := $(BUILD_DIR)/glutalk_cpp.o
RUST_LL := $(BUILD_DIR)/glutalk_rust.ll
GLU_OBJ := $(BUILD_DIR)/glutalk.o
GLU_BIN := $(BUILD_DIR)/glutalk

.PHONY: all clean run re

all: $(GLU_BIN)

$(BUILD_DIR):
	@mkdir -p $@

$(CPP_BC): $(CPP_SRC) | $(BUILD_DIR)
	$(CLANG) -std=c++20 -g -c -emit-llvm $< -o $@

$(CPP_OBJ): $(CPP_SRC) | $(BUILD_DIR)
	$(CLANG) -std=c++20 -g -c $< -o $@

$(RUST_LL): $(RUST_SRC) | $(BUILD_DIR)
	$(RUSTC) --crate-name glutalk_rust --crate-type=lib --emit=llvm-ir -g $< --out-dir $(BUILD_DIR)

$(GLU_OBJ): $(GLU_SRC) $(CPP_BC) $(RUST_LL) | $(BUILD_DIR)
	$(GLUC) $(GLU_SRC) -I $(BUILD_DIR) -c -o $@

$(GLU_BIN): $(GLU_OBJ) $(CPP_OBJ) $(RUST_LL)
	$(CLANG) -g -o $@ $^ $(GLU_STDLIB)

run: $(GLU_BIN)
	$(GLU_BIN)

clean:
	rm -rf $(BUILD_DIR)

re: clean all
