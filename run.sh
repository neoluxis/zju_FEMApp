#!/bin/bash

# once wrong exit then
set -e

cmake --build build

echo ----------------------------------------

x86_64-w64-mingw32-wine ./build/FemApp.exe "$@"
