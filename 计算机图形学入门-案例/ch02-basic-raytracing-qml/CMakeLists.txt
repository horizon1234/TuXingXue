cmake_minimum_required(VERSION 3.21)

project(Chapter02BasicRayTracing VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 Quick3D)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

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

set_source_files_properties(qml/Main.qml PROPERTIES
    QT_RESOURCE_ALIAS Main.qml
)

qt_add_qml_module(ch02_raytracing
    URI Chapter02RayTracing
    VERSION 1.0
    NO_GENERATE_QMLTYPES
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
    target_compile_definitions(ch02_raytracing PRIVATE NOMINMAX)
endif()

