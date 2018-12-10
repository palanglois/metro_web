CC=emcc
DEPS=external
OUT_DIR=./build_wasm
OUT_NAME=content_metro

all: $(OUT_NAME).html

$(OUT_NAME).html $(OUT_NAME).js $(OUT_NAME).wasm: main.cpp 
	$(CC) $< --emrun -s WASM=1 -o $(OUT_DIR)/$@ -I$(DEPS) -std=c++11 -DTINYPLY_IMPLEMENTATION=1 -O3 --shell-file html_template/shell_minimal.html -s NO_EXIT_RUNTIME=1  -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "stringToUTF8"]' -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 -DEMCC=1 -s TOTAL_MEMORY=167772160

.PHONY: clean

clean:
	rm $(OUT_DIR)/$(OUT_NAME)*
