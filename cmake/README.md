# FemApp CMake Configuration Files

This directory contains three independent, self-contained toolchain configurations.
Each file is atomic - include ONE file and get complete toolchain setup.

## Three Toolchain Options

### 1. WindowsOfficialQt.cmake
Windows with Official Qt (D:/Qt) + MinGW64 (完全独立)
- Self-contained: Official Qt6 + MinGW + Deployment
- Usage:
```cmake
include(${CMAKE_SOURCE_DIR}/cmake/WindowsOfficialQt.cmake)
```
- Command:
```bash
cmake .. -G Ninja -DQT_ROOT_DIR="D:/Qt/6.10.1/mingw_64" -DMINGW_ROOT="D:/Scoop/apps/msys2/current/mingw64"
```

### 2. WindowsMsys2Qt.cmake
Windows with MSYS2 Qt (D:/Scoop/apps/msys2) + MinGW64 (完全独立)
- Self-contained: MSYS2 Qt6 + MinGW + Deployment
- Usage:
```cmake
include(${CMAKE_SOURCE_DIR}/cmake/WindowsMsys2Qt.cmake)
```
- Command:
```bash
cmake .. -G Ninja -DQT_ROOT_DIR="D:/Scoop/apps/msys2/current/mingw64" -DMINGW_ROOT="D:/Scoop/apps/msys2/current/mingw64"
```

### 3. LinuxCross.cmake
Linux cross-compilation to Windows (完全独立)
- Self-contained: Cross-toolchain + Qt6 + Deployment
- Usage:
```cmake
include(${CMAKE_SOURCE_DIR}/cmake/LinuxCross.cmake)
```
- Command:
```bash
cmake .. -G Ninja
```

## Design Principles

✅ **Atomic**: Each file contains everything needed for that toolchain
✅ **Independent**: No dependencies between files
✅ **Non-interfering**: Switching toolchains only requires changing ONE include line
✅ **One Include**: Include one file = complete toolchain ready
```
