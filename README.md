# Viry3D
C++跨平台3D游戏引擎。

支持Windows、Android、iOS、macOS。

使用Unity3D类似的编程API。

Stack

邮箱：stackos@qq.com

QQ交流群：428374717

## 引擎功能
编程语言

    C++11

平台 & 3D API支持

    Vulkan, OpenGL ES 3.0
    Windows, Android, iOS, macOS

材质和Shader

    支持Vertex和Fragment shader，使用GLSL编写，以XML的格式组织结构
    在Shader中编写多Pass效果
    使用png, jpeg作为纹理文件存储格式

Mesh

    使用Unity3D导出场景和网格数据
    静态批次合并

动画

    使用Unity3D导出动画数据
    支持骨骼动画
        不同动作间的权重混合
        4骨骼权重蒙皮
        蒙皮硬件加速
    基于贝塞尔曲线的的AnimationCurve

场景

    GameObject-Component编程模型
    基于Transform的树形结构
    摄像机视口裁剪

效果

    HDR渲染管线
    延迟着色管线
    Directional, spot, point光源
    Shadow mapping，支持CSM
    Lightmap
    Camera组件
    ImageEffect系统，便捷的组织全屏后处理
    Bloom, Global Fog, Highlighting效果
    粒子系统
    Skybox
    Terrain渲染

物理

    集成Bullet Physics引擎
    BoxCollider
    MeshCollider
    TerrainCollider

UI

    UISprite绘制图片
    UILabel绘制文本
    批次自动合并

音频

    集成OpenAL，全平台使用统一接口
    支持wav，mp3（流式解码）格式

输入

    鼠标、键盘、触摸事件处理

其它

    文件IO
    UTF8、UTF32字符串编码
    数学库

## 游戏截图

---

![](https://github.com/stackos/Viry3D/blob/master/app/src/AppFlappyBird.png?raw=true)