cmake_minimum_required(VERSION 3.16)

project(pacman VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)

find_package(OpenGL REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    PacmanWidget.cpp
    PacmanWidget.h
    Ghost.cpp
    Ghost.h
    Constants.h
)

qt_add_executable(pacman
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(pacman PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    OpenGL::GL
    OpenGL::GLU
)

# Suppress legacy OpenGL deprecation warnings (macOS)
target_compile_definitions(pacman PRIVATE GL_SILENCE_DEPRECATION)

qt_finalize_executable(pacman)
