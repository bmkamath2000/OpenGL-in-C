# Pacman Net

A distributed Pacman game implementation with a Qt/C++ client, a C++ server, RabbitMQ message queuing, and Google Protocol Buffers for event serialization.

## Overview
![
    HIGH LEVEL ARCHITECTURE
](rabbitmq_protobuf_pacman_architecture.svg)

- `client/` runs the Pacman game UI, publishes game events to RabbitMQ, and sends user input/events to the server.
- `server/` logs events from message queues and can process game-related messages.
- `proto/` contains the Protocol Buffers schema and generated C++ protobuf sources.
- `shared/` contains shared game constants and logic used by both client and server.

## Architecture

1. The Qt-based client starts the game and publishes event messages to RabbitMQ.
2. The server subscribes to the RabbitMQ queue, receives game event messages, and logs them.
3. Messages are serialized using Google Protocol Buffers via the `.proto` definitions in `proto/pacman.proto`.
4. RabbitMQ is expected to run in Docker for local development.

## Prerequisites

- CMake
- A C++ compiler supporting C++11 or later
- Qt development libraries (for the client)
- Google Protocol Buffers
- RabbitMQ
- Docker (for running RabbitMQ)

## RabbitMQ Setup

This project expects RabbitMQ to be available in Docker. A simple command to start RabbitMQ locally:

```powershell
docker run -d --name pacman-rabbitmq -p 5672:5672 -p 15672:15672 rabbitmq:3-management
```

- Access the RabbitMQ management UI at `http://localhost:15672`
- Default credentials: `guest` / `guest`

## Build Instructions

### 1. Generate protobuf sources

If protobuf sources are not already generated, run the protobuf compiler on `proto/pacman.proto`:

```powershell
protoc --cpp_out=proto proto/pacman.proto
```

This writes `pacman.pb.h` and `pacman.pb.cc` to the `proto/` directory.

### 2. Build using per-project scripts

Each subproject includes a helper script to clean and build its `build` directory.

#### Client build script: `client/script.bat`

```bat
cd C:\Users\kamat\OneDrive\Documents\GitHub\OpenGL-in-C\pacman_net\client
rmdir /s /q build
mkdir build
cd build

cmake .. -G "MinGW Makefiles" ^
 -DCMAKE_C_COMPILER=C:/Qt/Tools/mingw1120_64/bin/gcc.exe ^
 -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1120_64/bin/g++.exe ^
 -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingw1120_64/bin/mingw32-make.exe ^
 -DCMAKE_PREFIX_PATH="C:/Qt/6.4.2/mingw_64;C:/protobuf-qt;C:/amqpcpp-qt" ^
 -DPROTOBUF_ROOT=C:/protobuf-qt ^
 -DCMAKE_BUILD_TYPE=Release

cmake --build . --config Release
```

#### Server build script: `server/script.bat`

```bat
cd C:\Users\kamat\OneDrive\Documents\GitHub\OpenGL-in-C\pacman_net\server
rmdir /s /q build
mkdir build
cd build

cmake .. -G "MinGW Makefiles" ^
-DCMAKE_C_COMPILER=C:/Qt/Tools/mingw1120_64/bin/gcc.exe ^
-DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1120_64/bin/g++.exe ^
-DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingw1120_64/bin/mingw32-make.exe ^
-DCMAKE_PREFIX_PATH="C:/protobuf-qt;C:/amqpcpp-qt" ^
-DProtobuf_INCLUDE_DIR=C:/protobuf-qt/include ^
-DProtobuf_LIBRARY=C:/protobuf-qt/lib/libprotobuf.a ^
-DProtobuf_PROTOC_EXECUTABLE=C:/protobuf-qt/bin/protoc.exe ^
-DAMQPCPP_LIB=C:/amqpcpp-qt/lib/libamqpcpp.a ^
-DAMQPCPP_INCLUDE_DIR=C:/amqpcpp-qt/include ^
-DCMAKE_BUILD_TYPE=Release

cmake --build .
```

### 3. Build all projects from root

The root `script.bat` file runs the server and client launch sequence after building the subprojects.

#### Root `script.bat`

```bat
@echo off

REM Start RabbitMQ if not already running
REM docker run -it --rm --name rabbitmq -p 5672:5672 -p 15672:15672 rabbitmq:4-management

REM Set PATH for Qt and MinGW
set PATH=C:\Qt\6.4.2\mingw_64\bin;C:\Qt\Tools\mingw1120_64\bin;%PATH%

REM Start server in a NEW window (non-blocking)
start "Pacman Server" cmd /k "cd C:\Users\kamat\OneDrive\Documents\GitHub\OpenGL-in-C\pacman_net\server\build && .\pacman_server.exe 127.0.0.1 5672 guest guest"

REM Wait 2 seconds for server to connect to RabbitMQ before starting client
timeout /t 2 /nobreak

REM Deploy Qt DLLs for client (only needed once)
cd C:\Users\kamat\OneDrive\Documents\GitHub\OpenGL-in-C\pacman_net\client\build

C:\Qt\6.4.2\mingw_64\bin\windeployqt.exe pacman_client.exe

copy C:\Qt\Tools\mingw1120_64\bin\libgcc_s_seh-1.dll . 2>nul
copy C:\Qt\Tools\mingw1120_64\bin\libstdc++-6.dll . 2>nul
copy C:\Qt\Tools\mingw1120_64\bin\libwinpthread-1.dll . 2>nul
copy C:\amqpcpp-qt\bin\*.dll . 2>nul
copy C:\protobuf-qt\bin\*.dll . 2>nul

REM Start client
.\pacman_client.exe 127.0.0.1 5672 guest guest
```

## Run

### Start RabbitMQ

Make sure RabbitMQ is running and accessible on `localhost:5672`.

### Launch from root script

The root `script.bat` will start the server in a new window, deploy required Qt runtime DLLs for the client, and launch the client:

```powershell
cd c:\Users\kamat\OneDrive\Documents\GitHub\OpenGL-in-C\pacman_net
script.bat
```

### Run manually if needed

```powershell
cd server\build
./pacman_server.exe 127.0.0.1 5672 guest guest

cd ..\..\client\build
./pacman_client.exe 127.0.0.1 5672 guest guest
```
![Running Client](<Screenshot 2026-05-09 115909.png>)
## Project Layout

- `client/` - Qt client application and RabbitMQ publisher logic
- `server/` - server application and RabbitMQ consumer/logger logic
- `proto/` - Protocol Buffers definitions and generated sources
- `shared/` - common headers/source shared by client and server

## Notes

- Game events are serialized with protobuf and routed through RabbitMQ queues.
- The server is responsible for logging and optionally processing game events.
- The client is responsible for running the game UI and publishing event messages.

## Troubleshooting

- If RabbitMQ cannot connect, verify Docker is running and the port mapping is correct.
- If protobuf generation fails, confirm `protoc` is installed and in `PATH`.
- Ensure Qt libraries are available to build the client.

## License

This repository does not specify a license. Add one if needed.
