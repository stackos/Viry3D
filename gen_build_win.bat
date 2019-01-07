@echo off
if exist build/win_x64 (
    cd build/win_x64
) else (
    mkdir build
    cd build
    mkdir win_x64
    cd win_x64
)
cmake ../../ -G "Visual Studio 15 2017 Win64" -DTarget=Windows -DArch=x64
cd ../../
pause