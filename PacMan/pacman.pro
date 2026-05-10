QT += core gui opengl openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = pacman
TEMPLATE = app

SOURCES += \
    main.cpp \
    Ghost.cpp \
    PacmanWidget.cpp

HEADERS += \
    Ghost.h \
    Constants.h \
    PacmanWidget.h

# Link OpenGL libraries
unix: LIBS += -lGL -lGLU
win32: LIBS += -lopengl32 -lglu32

# Suppress deprecation warnings for legacy OpenGL
DEFINES += GL_SILENCE_DEPRECATION
