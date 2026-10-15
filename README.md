# TuXingXue

《计算机图形学入门：3D 渲染指南》的精华笔记与 Qt/C++、QML 交互案例。

## 当前内容

```text
计算机图形学入门-精华笔记/
  第01章-基础入门概念.md
  第02章-基础光线追踪.md
  assets/

计算机图形学入门-案例/
  ch02-basic-raytracing-qml/
```

第 2 章案例采用以下分工：

- C++ 在 CPU 上实现 `CanvasToViewport`、射线与球体求交以及逐像素光线追踪。
- QML 实现界面、球体选择和坐标控制。
- Qt Quick 3D 显示相机、视口与球体的三维空间关系。

## 构建第 2 章案例

要求 Qt 6.5 或更高版本，并安装 Qt Quick、Qt Quick Controls 和 Qt Quick 3D。

当前工程路径包含中文。Windows 下部分 Qt 工具不能稳定处理中文构建路径，因此案例提供了自动盘符映射脚本：

```powershell
cd .\计算机图形学入门-案例\ch02-basic-raytracing-qml
powershell -ExecutionPolicy Bypass -File .\build.ps1
.\build-ascii\ch02_raytracing.exe
```

也可以使用案例目录中的 `Makefile` 快速构建和运行。Qt MinGW 环境提供的命令名称通常是 `mingw32-make`；如果系统已配置 `make`，可将下面命令中的 `mingw32-make` 替换为 `make`。

```powershell
cd .\计算机图形学入门-案例\ch02-basic-raytracing-qml
mingw32-make run
```

常用目标：

```text
mingw32-make build   # 只配置并编译
mingw32-make run     # 停止旧进程、编译并运行
mingw32-make stop    # 停止正在运行的程序
mingw32-make clean   # 删除生成的构建目录
mingw32-make help    # 查看帮助
```

两本原始 PDF 仅作为本地学习资料使用，不包含在仓库中。
