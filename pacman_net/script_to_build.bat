# Script to Build
# Builds AMQP-CPP and Protobuf
cd ..
git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
cd AMQP-CPP

rmdir build
mkdir build
cd build

# 3. Generate the Visual Studio solution files
cmake ..

# 4. Compile specifically for Release mode
cmake --build . --config Release

cd ..\..

git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf
git checkout v21.12
git submodule update --init --recursive
rmdir /s /q build
mkdir build
cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=C:/Qt/Tools/mingw1120_64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1120_64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingw1120_64/bin/mingw32-make.exe -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=C:/protobuf-qt

cmake --build . -j4
cmake --install .
cd ../..

