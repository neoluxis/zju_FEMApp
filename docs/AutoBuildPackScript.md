# 自动构建打包脚本说明

脚本路径：`<repo_root>/build.sh`

## Usage

可通过环境变量 `BUILD_DIR` 和 `PACK_DIR` 设置编译文件的目录和打包文件的目录，
也可以通过参数 `--build-dir` 和 `--pack-dir` 设置
默认构建路径为 `build`，打包路径 `package`

**帮助信息**：

```bash
./build.sh --help
```

**设置、构建并打包**

```bash
[env BUILD_DIR=build PACK_DIR=package] ./build.sh [--build-dir=build] [--pack-dir=package]
```

```bash
# 使用默认参数
./build.sh
```

**构建**：

```bash
./build.sh --build
```

**打包**：

```bash
./build.sh --package
```

---

默认每次编译都会清理之前的编译，可以通过 `--no-clean` 来取消清理。如果代码修改较少，不需要覆盖编译，可以通过此选项加快打包速度

可以使用 `--clean` 参数来强制清理，`--clean-build`、`--clean-package` 和 `--clean-zip` 用来单独清理构建、打包目录和过去打包的文件
