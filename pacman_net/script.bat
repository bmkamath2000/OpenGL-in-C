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