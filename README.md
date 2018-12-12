# Web Metro Distance

## Compiling instructions

Web assembly version of the metro distance.

This code can be compiled in WebAssembly thanks to 
[emscripten](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html).

First install emscripten with the 
[installation instructions](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html).

Then clone this repo : 

`git clone https://github.com/palanglois/metro_web.git`

Go to its directory : 

`cd metro_web`

Build with the Makefile : 

`make`

The resulting html + js + wasm files are in the folder build_wasm.
The resulting html page can be displayed with Firefox for example : 

`firefox build_wasm/content_metro.html`

## Compiling as a stand-alone C++ executable

A CMakeLists.txt is provided in order to compile the program as a stand-alone C++ executable. 
It does not compile the program in WebAssembly.

## Note about parallalization

This application has a multi-processor implementation which can be activated by changing the flag 
`PARALLEL=0` to `PARALLEL=1` is the Makefile (and execute cmake with the `-DBUILD_PARALLEL_CODE=1` 
option). However, as of today (Dec. 12th, 2018), most internet browsers have stopped 
supporting multi-threaded parallel computing because of the SPECTRE vulnerability (read more 
[here](https://kripken.github.io/emscripten-site/docs/porting/pthreads.html)).
