cmake_minimum_required(VERSION 3.16)

project(marble_solitaire VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

qt_add_executable(marble_solitaire
    MANUAL_FINALIZATION
    main.cpp
    MarbleWidget.cpp
    MarbleWidget.h
)

target_link_libraries(marble_solitaire PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
    OpenGL::GLU
)

target_compile_definitions(marble_solitaire PRIVATE GL_SILENCE_DEPRECATION)

qt_finalize_executable(marble_solitaire)
