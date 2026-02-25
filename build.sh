#!/bin/bash

rm -fr build package

cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw64.cmake -DFEMAPP_ADD_WIN32=ON -DCMAKE_BUILD_TYPE=Release

cmake --build build -j


mkdir -pv package
mkdir -pv package/translations

cp -r build/FemApp.exe package/
cp -r /usr/x86_64-w64-mingw32/lib/qt6/plugins/* package
cp -r translations/*.qm package/translations

env DLLPATH=/usr/x86_64-w64-mingw32/bin:/usr/x86_64-w64-mingw32/lib:./build \
x86_64-w64-mingw32-dllcopy package/FemApp.exe ./dll.txt --verbose -r

zip -j femapp_package_v$(date +%Y%m%d).zip package/*
