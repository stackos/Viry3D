#! /bin/bash
if [ ! -d "build/ios" ]; then
    mkdir -p build/ios
fi
cd build/ios
cmake ../../ -G "Xcode" -DTarget=iOS
cd ../../
