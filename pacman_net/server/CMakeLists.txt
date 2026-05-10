cmake_minimum_required(VERSION 3.16)
project(pacman_server VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Force CMake to ignore system PATH when searching for packages ─────────────
# Prevents CMake from finding MySQL's libprotobuf.lib or other stray installs.
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)

# ── Protobuf ──────────────────────────────────────────────────────────────────
if(DEFINED PROTOBUF_ROOT)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${PROTOBUF_ROOT}")
    set(Protobuf_INCLUDE_DIRS  "${PROTOBUF_ROOT}/include")
    set(Protobuf_PROTOC_EXECUTABLE "${PROTOBUF_ROOT}/bin/protoc.exe")
    if(WIN32)
        set(Protobuf_LIBRARIES "${PROTOBUF_ROOT}/lib/libprotobuf.lib")
    else()
        set(Protobuf_LIBRARIES "${PROTOBUF_ROOT}/lib/libprotobuf.a")
    endif()
endif()

find_package(Protobuf REQUIRED)

message(STATUS "Protobuf version   : ${Protobuf_VERSION}")
message(STATUS "Protobuf includes  : ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf libraries : ${Protobuf_LIBRARIES}")
message(STATUS "protoc executable  : ${Protobuf_PROTOC_EXECUTABLE}")

# ── Generate .pb.cc / .pb.h ───────────────────────────────────────────────────
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../proto/pacman.proto")
set(PROTO_OUT  "${CMAKE_CURRENT_BINARY_DIR}")
set(PROTO_CC   "${PROTO_OUT}/pacman.pb.cc")
set(PROTO_H    "${PROTO_OUT}/pacman.pb.h")

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
if(NOT AMQPCPP_LIB)
    find_library(AMQPCPP_LIB
        NAMES amqpcpp libamqpcpp
        HINTS "${CMAKE_PREFIX_PATH}"
        PATH_SUFFIXES lib
        REQUIRED
    )
endif()
if(NOT AMQPCPP_INCLUDE_DIR)
    find_path(AMQPCPP_INCLUDE_DIR
        NAMES amqpcpp.h
        HINTS "${CMAKE_PREFIX_PATH}" "C:/amqpcpp-qt"
        PATH_SUFFIXES include
    )
endif()
message(STATUS "amqpcpp lib    : ${AMQPCPP_LIB}")
message(STATUS "amqpcpp include: ${AMQPCPP_INCLUDE_DIR}")

if(WIN32)
    set(AMQP_EXTRA_LIBS ws2_32)
endif()

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(pacman_server
    main.cpp
    PacmanServer.cpp
    PacmanServer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/Ghost.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/Ghost.h
    ${PROTO_CC}
    ${PROTO_H}
)

target_include_directories(pacman_server PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    ${Protobuf_INCLUDE_DIRS}
    ${AMQPCPP_INCLUDE_DIR}
)

find_package(OpenGL REQUIRED)

target_compile_definitions(pacman_server PRIVATE PACMAN_SERVER_BUILD)

target_link_libraries(pacman_server PRIVATE
    ${Protobuf_LIBRARIES}
    ${AMQPCPP_LIB}
    ${AMQP_EXTRA_LIBS}
    OpenGL::GL
    OpenGL::GLU
)
