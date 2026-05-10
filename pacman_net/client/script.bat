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