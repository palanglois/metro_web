# User parameters
OUT_DIR=./build_wasm
#OUT_DIR=/home/langlois/Documents/perso-site
PARALLEL=0
OUT_NAME=content_metro

# Static parameters
CC=emcc
DEPS=external

# C++ related flags
EMCC_FLAGS = -std=c++11 -DTINYPLY_IMPLEMENTATION=1 -O3 -DEMCC=1

# EMCC html template file
EMCC_FLAGS += --shell-file html_template/shell_minimal.html

# EMCC specific flags
EMCC_FLAGS += -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "stringToUTF8"]'
EMCC_FLAGS += -s EXPORTED_FUNCTIONS="['_free', '_malloc']"
# EMCC_FLAGS += -s DISABLE_EXCEPTION_CATCHING=0 -s ASSERTIONS=1 # For debug

# Single-threaded/Multi-threaded flags
ifeq ($(PARALLEL), 1)
  EMCC_FLAGS += -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8 -s TOTAL_MEMORY=335544320 -DIS_PARALLEL=1
else
  EMCC_FLAGS += -s TOTAL_MEMORY=167772160 -s ALLOW_MEMORY_GROWTH=1
endif

all: $(OUT_NAME).html

$(OUT_NAME).html $(OUT_NAME).js $(OUT_NAME).wasm: main.cpp 
	$(CC) $< -s WASM=1 -o $(OUT_DIR)/$@ -I$(DEPS) $(EMCC_FLAGS)

.PHONY: clean

clean:
	rm $(OUT_DIR)/$(OUT_NAME)* && if [ $(PARALLEL) = 1 ]; then rm $(OUT_DIR)/pthread-main.js; fi
