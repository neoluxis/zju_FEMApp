#!/bin/bash


if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR=build
fi
if [ -z "$PACK_DIR" ]; then
    PACK_DIR=package
fi

rm -fr $BUILD_DIR $PACK_DIR

cmake -B $BUILD_DIR -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw64.cmake -DFEMAPP_ADD_WIN32=ON -DCMAKE_BUILD_TYPE=Release

cmake --build $BUILD_DIR -j


mkdir -pv $PACK_DIR
mkdir -pv $PACK_DIR/translations

cp -r $BUILD_DIR/FemApp.exe $PACK_DIR/
cp -r /usr/x86_64-w64-mingw32/lib/qt6/plugins/* $PACK_DIR
cp -r translations/*.qm $PACK_DIR/translations

env DLLPATH=/usr/x86_64-w64-mingw32/bin:/usr/x86_64-w64-mingw32/lib:$BUILD_DIR \
x86_64-w64-mingw32-dllcopy $PACK_DIR/FemApp.exe ./dll.txt --verbose -r

zip -r femapp_package_v$(date +%Y%m%d).zip $PACK_DIR/*
