@echo off
if not exist build\uwp_x64 (
    mkdir build\uwp_x64
)
cd build\uwp_x64
cmake ..\..\ -G "Visual Studio 15 2017 Win64" -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 -DTarget=UWP -DArch=x64
cd ..\..\
pause