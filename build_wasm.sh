#! /bin/bash
if [ ! -d "build/wasm" ]; then
    mkdir -p build/wasm
fi
cd build/wasm
emconfigure cmake ../../ -DTarget=WASM
emmake make
cp -rf ../../app/project/web/Assets ./
cp -rf ../../app/project/web/audio ./
cp -rf ../../app/project/web/texture ./
cp -f ../../app/project/web/index.html index.html
cp -f ../../app/project/web/index.js index.js
emcc -O3 libViry3DApp.a libViry3D.a libViry3DDep.a \
    -o viry3d.js \
    -s NO_EXIT_RUNTIME=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' -s ALLOW_MEMORY_GROWTH=1 \
    -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s USE_WEBGL2=0 \
    --preload-file Assets
cd ../../
