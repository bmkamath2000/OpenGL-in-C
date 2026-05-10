cmake_minimum_required(VERSION 3.16)
project(pacman_client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# ── MSVC requires /Zc:__cplusplus for Qt to detect C++17 correctly ────────────
if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

# ── Force CMake to ignore system PATH when searching for packages ─────────────
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)

# ── Qt ────────────────────────────────────────────────────────────────────────
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

# ── Protobuf + bundled Abseil ─────────────────────────────────────────────────
# ── Protobuf Logic ────────────────────────────────────────────────────────────

if(DEFINED PROTOBUF_ROOT)
    set(Protobuf_DIR "${PROTOBUF_ROOT}/lib/cmake/protobuf")
    set(utf8_range_DIR "${PROTOBUF_ROOT}/lib/cmake/utf8_range")
    set(absl_DIR "${PROTOBUF_ROOT}/lib/cmake/absl")
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${PROTOBUF_ROOT}")
    # Force the include path if CONFIG mode fails to set it correctly
    if(Protobuf_FOUND AND DEFINED PROTOBUF_ROOT)
        set(Protobuf_INCLUDE_DIRS "${PROTOBUF_ROOT}/include")
    endif()
    set(Protobuf_PROTOC_EXECUTABLE "${PROTOBUF_ROOT}/bin/protoc.exe")
    if(WIN32)
        set(Protobuf_LIBRARIES "${PROTOBUF_ROOT}/lib/libprotobuf.lib")
    else()
        set(Protobuf_LIBRARIES "${PROTOBUF_ROOT}/lib/libprotobuf.a")
    endif()
    message(STATUS "Using manual Protobuf root: ${PROTOBUF_ROOT}")
endif()

set(utf8_range_DIR "C:/protobuf-qt/lib/cmake/utf8_range")
set(absl_DIR "C:/protobuf-qt/lib/cmake/absl")

find_package(absl REQUIRED CONFIG)
find_package(utf8_range REQUIRED CONFIG)
find_package(Protobuf REQUIRED CONFIG)


if(NOT Protobuf_FOUND)
    message(FATAL_ERROR
        "Protobuf not found.\n"
        "Option 1 - vcpkg:  set CMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake\n"
        "Option 2 - manual: set PROTOBUF_ROOT=C:/path/to/protobuf/install\n"
        "Option 3 - winget: winget install Google.Protobuf  (then set PROTOBUF_ROOT)\n"
    )
endif()

message(STATUS "Protobuf version   : ${Protobuf_VERSION}")
message(STATUS "Protobuf includes  : ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf libraries : ${Protobuf_LIBRARIES}")
message(STATUS "protoc executable  : ${Protobuf_PROTOC_EXECUTABLE}")

# Protobuf v26+ bundles abseil — link all absl libs found in the install prefix
file(GLOB ABSL_LIBS
    "C:/protobuf-qt/lib/libabsl_*.a"
)
if(ABSL_LIBS)
    message(STATUS "Found ${CMAKE_MATCH_0} abseil libs, linking them all")
else()
    message(STATUS "No abseil libs found — if linker errors occur, check C:/protobuf-qt/lib/")
endif()

# ── Generate .pb.cc / .pb.h from pacman.proto ─────────────────────────────────
set(PROTO_FILE   "${CMAKE_CURRENT_SOURCE_DIR}/../proto/pacman.proto")
set(PROTO_OUT    "${CMAKE_CURRENT_BINARY_DIR}")
set(PROTO_CC     "${PROTO_OUT}/pacman.pb.cc")
set(PROTO_H      "${PROTO_OUT}/pacman.pb.h")

# Use add_custom_command so regeneration happens automatically when .proto changes
add_custom_command(
    OUTPUT  ${PROTO_CC} ${PROTO_H}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
            --cpp_out=${PROTO_OUT}
            --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../proto
            pacman.proto
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf sources from pacman.proto"
)

# ── AMQP-CPP ─────────────────────────────────────────────────────────────────
# Can be overridden by passing -DAMQPCPP_LIB=... to cmake directly
if(NOT AMQPCPP_LIB)
    find_library(AMQPCPP_LIB
        NAMES amqpcpp libamqpcpp
        HINTS "${CMAKE_PREFIX_PATH}"
        PATH_SUFFIXES lib
        REQUIRED
    )
endif()
if(NOT AMQPCPP_LIB)
    message(FATAL_ERROR "amqpcpp not found. Build from source with Qt's MinGW and set -DAMQPCPP_LIB=")
endif()
message(STATUS "amqpcpp lib: ${AMQPCPP_LIB}")

# amqpcpp include dir — from same folder as the lib, or from source build
if(NOT AMQPCPP_INCLUDE_DIR)
    find_path(AMQPCPP_INCLUDE_DIR
        NAMES amqpcpp.h
        HINTS "${CMAKE_PREFIX_PATH}" "C:/amqpcpp-qt"
        PATH_SUFFIXES include
    )
endif()
message(STATUS "amqpcpp include: ${AMQPCPP_INCLUDE_DIR}")

# ── Executable ────────────────────────────────────────────────────────────────
qt_add_executable(pacman_client
    MANUAL_FINALIZATION
    main.cpp
    ClientWidget.cpp
    ClientWidget.h
    ${PROTO_CC}
    ${PROTO_H}
)

target_include_directories(pacman_client PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_PREFIX_PATH}/include
    ${AMQPCPP_INCLUDE_DIR}
)

target_sources(pacman_client PRIVATE
    main.cpp
    ClientWidget.cpp
    ClientWidget.h
    ${PROTO_CC}
    ${PROTO_H}
)

target_link_libraries(pacman_client PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets
    OpenGL::GL OpenGL::GLU
    protobuf::libprotobuf    # Use the namespaced target from CONFIG mode
    utf8_range::utf8_validity # Link this EXPLICITLY to fix the undefined reference
    ${ABSL_LIBS}
    ${AMQPCPP_LIB}
    ws2_32
)
# pthreads: on Windows with MinGW it's built-in; on Linux link explicitly
if(UNIX)
    target_link_libraries(pacman_client PRIVATE pthread)
endif()

target_compile_definitions(pacman_client PRIVATE
    GL_SILENCE_DEPRECATION
    $<$<BOOL:${WIN32}>:WIN32_LEAN_AND_MEAN NOMINMAX>
)

qt_finalize_executable(pacman_client)
