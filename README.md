![](https://raw.githubusercontent.com/stackos/Viry3D/master/app/bin/Assets/texture/logo720p.png)

# Viry3D
C++跨平台3D游戏引擎。

支持Windows、Android、iOS、macOS和Web(基于WebAssembly)。

Stack

邮箱：stackos@qq.com

QQ交流群：428374717

## Build
### Windows
* Visual Studio 2017
* app/project/win/app.sln

### Android
* Android Studio
* app/project/android
* 参照 [https://developer.android.google.cn/ndk/guides/graphics/getting-started.html](https://developer.android.google.cn/ndk/guides/graphics/getting-started.html) 编译shaderc
```
cd (your android sdk dir)\ndk-bundle\sources\third_party\shaderc
..\..\..\ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk \
    APP_STL:=c++_shared APP_ABI=armeabi-v7a APP_PLATFORM=android-18 libshaderc_combined
```
* Python(for copy assets cmd)

### iOS
* Xcode
* app/project/ios/app.xcodeproj

### macOS
* Xcode
* app/project/mac/app.xcodeproj

## 引擎功能
编程语言

    C++11

平台 & 3D API支持

    Vulkan, OpenGL ES 2.0/3.0
    Windows, Android, iOS, macOS, Web(基于WebAssembly)

Mesh

    使用Unity3D导出场景和网格数据
    包含材质和纹理导出

动画

    使用Unity3D导出动画数据
    支持骨骼动画
        不同动作间的权重混合
        4骨骼权重蒙皮
        蒙皮硬件加速
    基于贝塞尔曲线的的AnimationCurve

效果

    Camera
    Mesh Renderer
    SkinnedMesh Renderer
    Light
    Skybox
    Render To Texture
    FXAA
    PostEffect Blur
    Shadow Map

UI

    Canvas Renderer
    Sprite
    Label
    Freetype Font
    Button

输入

    鼠标、键盘、触摸事件处理

其它

    文件IO
    UTF8、UTF32字符串编码
    数学库


## 在线Demo
[http://www.viry3d.com/](http://www.viry3d.com/)
