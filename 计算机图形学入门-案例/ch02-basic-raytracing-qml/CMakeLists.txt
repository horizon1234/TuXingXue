cmake_minimum_required(VERSION 3.21)

project(Chapter02BasicRayTracing VERSION 1.0 LANGUAGES CXX)

# 代码使用结构化绑定和 std::clamp，因此最低要求 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SphereModel 和 SceneController 含 Q_OBJECT，由 CMake 自动调用 moc。
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Quick3D)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

# 主程序包含 QML 后端、CPU 光线追踪算法和 image provider。
qt_add_executable(ch02_raytracing
    src/main.cpp
    src/rayimageprovider.cpp
    src/rayimageprovider.h
    src/raytracer.cpp
    src/raytracer.h
    src/scenecontroller.cpp
    src/scenecontroller.h
    src/sphere.h
    src/spheremodel.cpp
    src/spheremodel.h
)

# 源文件保存在 qml/ 子目录，但资源别名放到模块根部，使
# engine.loadFromModule("Chapter02RayTracing", "Main") 能稳定找到 Main.qml。
set_source_files_properties(qml/Main.qml PROPERTIES
    QT_RESOURCE_ALIAS Main.qml
)

qt_add_qml_module(ch02_raytracing
    URI Chapter02RayTracing
    VERSION 1.0

    # C++ 对象通过 QQmlContext 暴露，没有使用 QML_ELEMENT，因此无需生成类型描述。
    NO_GENERATE_QMLTYPES

    # 当前模块均为动态 Qt 模块，不需要静态插件 import scan。关闭扫描还能规避
    # Qt 6.9 qmlimportscanner 在部分 Windows 中文路径下的编码缺陷。
    NO_IMPORT_SCAN
    QML_FILES
        qml/Main.qml
)

target_link_libraries(ch02_raytracing
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Quick3D
)

if(WIN32)
    # 避免 Windows SDK 的 min/max 宏破坏 std::clamp 等标准库调用。
    target_compile_definitions(ch02_raytracing PRIVATE NOMINMAX)
endif()

