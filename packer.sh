#!/bin/bash

rm -frv packaging
mkdir -pv packaging
cp -v build/FemApp.exe packaging/

x86_64-w64-mingw32-wine windeployqt6 ./packaging/FemApp.exe 
