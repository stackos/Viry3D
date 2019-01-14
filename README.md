![](https://raw.githubusercontent.com/stackos/Viry3D/master/app/bin/Assets/texture/logo720p.png)

# Viry3D

[![Join the chat at https://gitter.im/viry3d_community/Lobby](https://badges.gitter.im/viry3d_community/Lobby.svg)](https://gitter.im/viry3d_community/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

C++ 跨平台 3D 游戏引擎。

支持 Android、iOS、macOS、Windows、

UWP（Windows 通用平台）、

Web（基于 WebAssembly）。

邮箱：stackos@qq.com

QQ 交流群：428374717

## Build
### Windows
* Visual Studio 2017
* CMake 3.10.2 or higher
* `gen_build_win.bat` generate project in `build`

### UWP
* Visual Studio 2017
* CMake 3.10.2 or higher
* `gen_build_uwp.bat` generate project in `build`

### iOS
* Xcode
* CMake 3.10.2 or higher
* `gen_build_ios.sh` generate project in `build`

### macOS
* Xcode
* CMake 3.10.2 or higher
* `gen_build_mac.sh` generate project in `build`

### Android
* Android Studio
* CMake 3.10.2 or higher
* `app/project/android`
* 如使用 Vulkan ，参照 [https://developer.android.google.cn/ndk/guides/graphics/getting-started.html](https://developer.android.google.cn/ndk/guides/graphics/getting-started.html) 编译 shaderc
```
cd (your android sdk dir)\ndk-bundle\sources\third_party\shaderc
..\..\..\ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk \
    APP_STL:=c++_shared APP_ABI=armeabi-v7a APP_PLATFORM=android-18 libshaderc_combined
```
* Python(for copy assets cmd)

### WebAssembly
* Emscripten SDK, see [https://webassembly.org/getting-started/developers-guide/](https://webassembly.org/getting-started/developers-guide/)
* CMake 3.10.2 or higher
* `build_wasm.sh` build output in `build/wasm`

## 引擎功能
编程语言

    C++11

平台 & 3D API支持

    Vulkan、OpenGL ES 2.0/3.0
    Android、iOS、macOS、Windows、UWP（Windows 通用平台）、Web（基于 WebAssembly）

Mesh

    使用 Unity3D 导出场景和网格数据
    包含材质和纹理导出

动画

    使用 Unity3D 导出动画数据
    支持骨骼动画
        不同动作间的权重混合
        4 骨骼权重蒙皮
        蒙皮硬件加速
    基于贝塞尔曲线的的 AnimationCurve
    支持 BlendShape 动画

渲染

    Camera
    Mesh Renderer
    SkinnedMesh Renderer
    Light
    Lightmap
    Skybox
    Render To Texture
    FXAA
    MSAA
    PostEffect Blur
    Shadow Map
    Instancing
    SSAO
    VR
    PBR
    Compute Shader

UI

    Canvas Renderer
    Sprite
    Label
    Freetype Font
    Button
    Select Button
    Switch Button
    Line View
    Scroll View
    Slider
    Navigation Mesh 2D

输入

    鼠标、键盘、触摸事件处理

音频

    基于 OpenAL 的跨平台 3D 音频播放
    支持 wav、流式 mp3 格式

其它

    文件 IO
    UTF8、UTF32 字符串编码
    数学库

## 在线 Demo
[http://www.viry3d.com/](http://www.viry3d.com/)
