# TuXingXue

《计算机图形学入门：3D 渲染指南》的精华笔记与 Qt/C++、QML 交互案例。

## 当前内容

```text
计算机图形学入门-精华笔记/
  第01章-基础入门概念.md
  第02章-基础光线追踪.md
  第03章-光.md
  assets/

计算机图形学入门-案例/
  ch02-basic-raytracing-qml/
```

第 2、3 章共用同一个递进式案例，采用以下分工：

- C++ 在 CPU 上实现射线与球体求交、环境光、漫反射和镜面反射。
- QML 实现场景切换、球体选择和坐标控制。
- Qt Quick 3D 显示相机、视口、光源与球体的三维空间关系。

## 构建第 2、3 章案例

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

两本学习 PDF 通过 Git LFS 纳入仓库。首次克隆前请安装 Git LFS，并执行 `git lfs pull` 获取原始文件；资料的公开分发请确认符合相应版权许可。
