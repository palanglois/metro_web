CC=emcc
DEPS=external
OUT_DIR=./build_wasm
#OUT_DIR=/home/langlois/Documents/perso-site
OUT_NAME=content_metro
PARALLEL=0

EMCC_FLAGS = -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "stringToUTF8"]' -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 -DEMCC=1 -s TOTAL_MEMORY=167772160


ifeq ($(PARALLEL), 1)
  EMCC_FLAGS += -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8 -s TOTAL_MEMORY=335544320 -DIS_PARALLEL=1
else
  EMCC_FLAGS +=   -s ALLOW_MEMORY_GROWTH=1
endif

all: $(OUT_NAME).html

$(OUT_NAME).html $(OUT_NAME).js $(OUT_NAME).wasm: main.cpp 
	$(CC) $< --emrun -s WASM=1 -o $(OUT_DIR)/$@ -I$(DEPS) -std=c++11 -DTINYPLY_IMPLEMENTATION=1 -O3 --shell-file html_template/shell_minimal.html $(EMCC_FLAGS)

.PHONY: clean

clean:
	rm $(OUT_DIR)/$(OUT_NAME)* && if [ $(PARALLEL) ]; then rm $(OUT_DIR)/pthread-main.js; fi
