#! /bin/bash
if [ ! -d "build/mac" ]; then
    mkdir -p build/mac
fi
cd build/mac
cmake ../../ -G "Xcode" -DTarget=Mac
cd ../../
