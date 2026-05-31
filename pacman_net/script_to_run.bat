@echo off
setlocal enabledelayedexpansion

echo [1/4] Starting RabbitMQ in background...
docker run -d --rm --name rabbitmq -p 5672:5672 -p 15672:15672 rabbitmq:4-management

echo [2/4] Waiting for RabbitMQ to complete AMQP handshake readiness...
set /a timer=0
set /a max_timeout=30

:wait_loop
:: Check if RabbitMQ is fully ready to accept AMQP traffic
docker exec rabbitmq rabbitmq-diagnostics check_port_connectivity >nul 2>&1
if %errorlevel% equ 0 goto rabbitmq_ready

:: Increment timer and check for script timeout
set /a timer+=2
if !timer! gtr !max_timeout! (
    echo [ERROR] RabbitMQ took too long to start. Exiting script.
    pause
    exit /b 1
)

echo Waiting for AMQP port to accept handshakes... (!timer!/%max_timeout%s)
timeout /t 2 /nobreak >nul
goto wait_loop

:rabbitmq_ready
echo RabbitMQ is fully initialized and ready for connections!

REM Set PATH for Qt and MinGW
set PATH=C:\Qt\6.4.2\mingw_64\bin;C:\Qt\Tools\mingw1120_64\bin;%PATH%

echo [3/4] Starting Pacman Server in a new window...
start "Pacman Server" cmd /k "cd .\server\build && .\pacman_server.exe 127.0.0.1 5672 guest guest"

echo [4/4] Deploying Qt DLLs and starting client...
cd .\client\build

C:\Qt\6.4.2\mingw_64\bin\windeployqt.exe pacman_client.exe

copy C:\Qt\Tools\mingw1120_64\bin\libgcc_s_seh-1.dll . 2>nul
copy C:\Qt\Tools\mingw1120_64\bin\libstdc++-6.dll . 2>nul
copy C:\Qt\Tools\mingw1120_64\bin\libwinpthread-1.dll . 2>nul
copy C:\amqpcpp-qt\bin\*.dll . 2>nul
copy C:\protobuf-qt\bin\*.dll . 2>nul

echo Launching Pacman Client...
.\pacman_client.exe 127.0.0.1 5672 guest guest

endlocal
