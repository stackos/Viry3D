#! /bin/bash
if [ ! -d "build/mac" ]; then
    echo "dir not exist"
    mkdir -p build/mac
fi
cd build/mac
cmake ../../ -G "Xcode" -DTarget=Mac
cd ../../
