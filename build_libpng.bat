
@echo off
git submodule update --init External/src/libpng
mkdir External\build\libpng
cd External\build\libpng
cmake -DCMAKE_INSTALL_PREFIX=../../Windows -G "Visual Studio 17 2022" -A "x64" ../../src/libpng
cmake --build . --config Release --target install
popd
