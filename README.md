# FEM App

## Modules

- `FemApp`: 主程序入口与流程控制（配置加载、匹配、编辑流程编排）
- `ProjectControlWidget`: 项目配置 UI 子包（项目操作 + Dose/Focus/FEM/Text）
- `MultiPrjWs`: `.femmpw` 多项目工作空间模型与左侧工作区控件
- `Recent`: 最近打开历史记录（按访问时间排序）
- `FEMConfig`: `.fem` 配置解析、匹配与序列化
- `XLSXEditor`: 图片与描述编辑控件
- `MiniXLSX`: xlsx 读写基础能力
- `FileAssocLib`: Windows 文件关联支持

## Current Features

- 全局 `File` 菜单支持 `New -> Project/Workspace`、`Open`、`Recent`、`Save`、`Save As`。
- `Recent` 按时间排序，并按 `Workspace` / `Project` 分组展示，支持 `Clear` 一键清空。
- 新建项目/新建工作区（含默认项目）后进入“宽松配置期”：
    - 允许 `folder/filename/sheet` 多匹配或暂未匹配；
    - 修改文件夹/文件时优先刷新候选列表，不中断 UI；
    - 首次点击“刷新编辑器”后恢复严格检查与报错。
- 打开空 `.fem` / 空 `.femmpw` 文件时自动进入宽松配置期，便于从空文件开始定义。
- 工作区侧边栏精简为 `Config` 按钮，打开工作区配置窗口；配置窗口改为自动保存（修改即落盘）。

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
cp -rv ./build/{FemApp.exe,*.dll,platforms,translations} package
windeployqt6 package/FemApp.exe # Installed in system or in Wine or Copy from Windows
zip package.zip package -r
```
