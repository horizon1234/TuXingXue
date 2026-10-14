# 第 2 章：基础光线追踪演示

这是《计算机图形学入门：3D 渲染指南》第 2 章的交互测试程序。

程序严格实现本章范围内的算法：固定相机、固定视口、射线与球体求交、最近交点和纯色输出。光照、阴影和高光留到后续章节。

## 界面结构

- 左侧：Qt Quick 3D 教学场景，显示概念相机、视口平面和三个球体。
- 右侧：C++ 在 CPU 上逐像素生成的光线追踪画布。
- 底部：球体选择和 `X/Y/Z` 坐标控制。

在左侧选中球体后可以直接拖动其 `X/Y` 坐标，滚轮调整 `Z` 坐标。两侧会同步更新。

## 技术结构

```text
QML / Qt Quick 3D
  ├─ 三维教学场景
  ├─ 球体选择与位置控制
  └─ 光线追踪结果显示
             ↓
C++
  ├─ CanvasToViewport
  ├─ IntersectRaySphere
  ├─ TraceRay
  └─ QImage 像素输出
```

Qt Quick 3D 只负责左侧可视化。右侧图像由 `RayTracer` 按书中的算法计算，不使用 GPU 光栅化代替光线追踪。

## 构建

要求 Qt 6.5 或更高版本，并安装以下模块：

- Qt Quick
- Qt Quick Controls
- Qt Quick 3D

当前工作区路径包含中文。Qt 6.9.3 的 `moc` 和 `qmlimportscanner` 在部分 Windows 环境中不能正确处理该路径，因此项目提供了构建脚本，通过临时 ASCII 盘符完成构建，并在结束后自动解除映射。

在当前机器上执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
.\build-ascii\ch02_raytracing.exe
```

程序还提供截图验证参数：

```powershell
.\build-ascii\ch02_raytracing.exe --screenshot screenshot.png
```
