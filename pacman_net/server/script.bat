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