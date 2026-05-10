cmake_minimum_required(VERSION 3.16)
project(Robot_Qt LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

# MinGW 64-bit path
set(CMAKE_PREFIX_PATH "C:/Qt/6.4.2/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

add_executable(Robot_Qt
    main.cpp
    robowidget.h
    robowidget.cpp
    robo.c
    robo.h
)

target_link_libraries(Robot_Qt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    opengl32
    glu32
)

set_target_properties(Robot_Qt PROPERTIES WIN32_EXECUTABLE TRUE)
