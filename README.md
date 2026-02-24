# FEM App

## Build

Deps install

```bash
yay -Syu mingw-w64-toolchain mingw-w64
yay -S mingw-w64-cmake mingw-w64-extra-cmake-modules
yay -S mingw-w64-qt6-base mingw-w64-qt6-tools
yay -S mingw-w64-pugixml mingw-w64-zlib
yay -S mingw-w64-dllcopy # a bash tool to copy dll needed
```

Build

```bash
# Generate for Windows Debug
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw64.cmake -DFEMAPP_ADD_WIN32=ON

# Generate for Linux while skipping RegEdit Debug
cmake -B build -G Ninja -DFEMAPP_LINUX_BUILD_DEMO=ON

# Generate for Windows Release
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw64.cmake -DFEMAPP_ADD_WIN32=ON -DCMAKE_BUILD_TYPE=Release

# Generate for Linux while skipping RegEdit Release
cmake -B build -G Ninja -DFEMAPP_LINUX_BUILD_DEMO=ON -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j

# if build for Windows, you need to copy dlls to the exe folder to run
# after this command you can see all dll needed in build dir
env DLLPATH=/usr/x86_64-w64-mingw32/bin:./build x86_64-w64-mingw32-dllcopy build/FemApp.exe ./dll.txt --verbose -r
```

Test

- Linux
    ```bash
    ./build/FemApp ./assets/project.fem
    ```
- Windows
    ```bash
    x86_64-w64-mingw32-wine ./build/FemApp.exe ./assets/project.fem
    ```

Package

```bash
mkdir -pv package
cp -rv ./build/{FemApp.exe,*.dll,platforms,transla┌─[neolux@neoluxeische] - [~/workspace/FEM/FemApp/XLSXEditor] - [2026-02-24 10:44:37]
└─[0] <git:(main 952a97d) > ./build/XLSXEditor_test assets/test.xlsx
Running in REAL-DELETE mode (will delete pictures and definitions)
OpenXLSXWrapper::open error: Path xl/sharedStrings.xml does not exist in zip archive.tions} package
windeployqt6 package/FemApp.exe # Installed in system or in Wine or Copy from Windows
zip package.zip package -r
```
