CC=emcc
DEPS=external
OUT_DIR=./build_wasm

content_metro.html content_metro.js content_metro.wasm: main.cpp 
	$(CC) $< --emrun -s WASM=1 -o $(OUT_DIR)/$@ -I$(DEPS) -std=c++11 -DTINYPLY_IMPLEMENTATION=1 -O3 --shell-file html_template/shell_minimal.html -s NO_EXIT_RUNTIME=1  -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall"]' -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 -DEMCC=1 -s TOTAL_MEMORY=167772160
