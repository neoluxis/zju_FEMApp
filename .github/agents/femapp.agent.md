---
description: FEM软件开发任务
tools: ['insert_edit_into_file', 'replace_string_in_file', 'create_file', 'run_in_terminal', 'get_terminal_output', 'get_errors', 'show_content', 'open_file', 'list_dir', 'read_file', 'file_search', 'grep_search', 'validate_cves', 'run_subagent', 'semantic_search']
---

# FEM软件

## 基本要求
（1）用Qt C++开发，软件尽量小，耗用尽量少的资源。中间结果优先存为文件，使用完的内存尽快释放掉。
（2）数据与代码分离，附带好所有依赖的资源，方便在其他电脑快速配置环境。

## 工具链安装位置

QT5 static MINGW64: D:\Scoop Applications\apps\msys2\current\mingw64\qt5-static

QT6 static MINGW64: D:\Scoop Applications\apps\msys2\current\mingw64\qt6-static

QT5 MINGW64: D:\Scoop Applications\apps\msys2\current\mingw64

QT6 MINGW64: D:\Scoop Applications\apps\msys2\current\mingw64

## 代码风格

1. OOP
2. include 和 src 分离， 其中软件的include域名应该为 cc/neolux/fem
3. 代码注释完整，函数和类的注释使用Doxygen风格
4. 变量命名使用驼峰命名法

## FEM 项目配置

文件后缀名为 `.fem`

```
//是注释符号

在当前文件夹检索文件夹名，有重复的或者检索不到则报错，* 表示通配符
folder = *data*
//在指定的文件夹检索文件名，有重复的或者检索不到则报错
filename = *AT00580.27*  
//在excel中检索sheet，有重复的或者检索不到则报错
sheet = *DNo.3*
//dose的range和Excel对应的列
dose={"mode":"LowHigh", "unit":"mJ/cm2",
"center":0, "step":0.05, "no": 23,
"cols":"B:K"}

//focus的range和Excel对应的行
focus={"mode":"NegPos2", "unit":"μm",
"center":0, "step":0.03, "no": 29,
"rows":"3:60"
}

//fem的要求
fem={"mode":"Focus2DoseLinear", "unit":"mJ/cm2",
"target":80, "spec":5}
```

任务配套文件夹，文件夹放数据和输出结果。数据和输出结果后续对接 Python 程序，此处暂不考虑。

## 模块初步设计

- FileAssociation: 在运行软件时，检查windows系统是否已经将 `.fem` 文件关联到该软件。
如若没有，则自动关联。同时应该支持取消关联
- FEMConfigLoader: 负责加载 `.fem` 文件，识别数据，并可以编辑项目配置



任务一：加载 `.fem` 文件，根据 `.fem` 识别数据，并可以编辑。

以assets/AT00229.01-s25-AA（IMAGE）E STEP 0.05.xlsx
SO25(DNo.6)MS为例

任务二：可以选择图片

每个图片可以单独点击放大。
数据不好的数据可以叉掉，叉掉的数据直接被淘汰掉。
选择结果存为图片和选过的数据。
