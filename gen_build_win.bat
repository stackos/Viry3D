@echo off
if exist build (
    ::rmdir /s/q build
    cd build/win
) else (
    mkdir build
    cd build
    mkdir win
    cd win
)
cmake ../../ -G "Visual Studio 15 2017 Win64" -DTarget=Windows -DArch=x64
cd ../../