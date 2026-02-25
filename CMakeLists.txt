cmake_minimum_required(VERSION 3.10)

option(FEMAPP_LINUX_BUILD_DEMO "Build demo on Linux, to skip some windows api that have tested such as REGEDITs or so" OFF)
option(FEMAPP_ADD_WIN32 "Add WIN32 to prevent console on Windows, but also hide logs" OFF)
# Toolchain must be included BEFORE project()
if(NOT FEMAPP_LINUX_BUILD_DEMO)
    include(${CMAKE_SOURCE_DIR}/cmake/LinuxCross.cmake)
endif()


if(FEMAPP_LINUX_BUILD_DEMO)
    message("Building a quick demo skipping Windows APIs processings")
    add_definitions(-DFEMAPP_SKIP_WINDOWS_API)
endif(FEMAPP_LINUX_BUILD_DEMO)




# Ensure CMAKE_PREFIX_PATH is set correctly for find_package
list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT_DIR}")


project(FemApp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets LinguistTools)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加 FEMConfig 库子项目
add_subdirectory(FEMConfig)
if(FEMAPP_LINUX_BUILD_DEMO)
    set(MiniXLSX_BUILD_TEST OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory(MiniXLSX)
add_subdirectory(FileAssocLib)
add_subdirectory(XLSXEditor)
add_subdirectory(ProjectControlWidget)
add_subdirectory(Recent)

# Add source files
set(SOURCES
    fem/src/main.cpp
    fem/src/xlsx_proc.cpp
    fem/src/femapp.cpp
    fem/src/xlsx_editor_module.cpp
)

set(HEADERS
    fem/include/
)

#set(UI_HEADERS
#        fem/include/ui_mainwindow.h
#)
set(UI_FILES
    fem/src/mainwindow.ui
)


FILE(GLOB TRANSLATION_TS translations/*.ts)

# Create executable with WIN32 flag to prevent console window on Windows
if(FEMAPP_ADD_WIN32)
    add_executable(FemApp WIN32 ${SOURCES})
else(FEMAPP_ADD_WIN32)
    add_executable(FemApp ${SOURCES})
endif(FEMAPP_ADD_WIN32)


# For Windows cross-compilation, allow multiple symbol definitions
# This is needed because multiple DLLs statically link libstdc++
target_link_options(FemApp PRIVATE -Wl,--allow-multiple-definition)

# Include directories
target_include_directories(FemApp PRIVATE
    ${HEADERS}
    ${CMAKE_SOURCE_DIR}/FEMConfig/include
    ${CMAKE_SOURCE_DIR}/FileAssocLib/include
    ${CMAKE_SOURCE_DIR}/XLSXEditor/include
    ${CMAKE_SOURCE_DIR}/ProjectControlWidget/include
    ${CMAKE_SOURCE_DIR}/Recent/include
)

# Link libraries
target_link_libraries(FemApp PRIVATE
    Qt6::Core
    Qt6::Widgets
    FEMConfig
    # OpenXLSX
    MiniXLSX
    XLSXEditor
    ProjectControlWidget
    Recent
)

if(NOT FEMAPP_LINUX_BUILD_DEMO)
    target_link_libraries(FemApp PRIVATE
        FileAssocLib
        shlwapi
        shell32
    )
endif()

# 生成 / 更新 .ts 文件 (use Qt-provided lupdate)
# 扫描主项目和 XLSXEditor 的所有源文件
add_custom_target(update_translations
    COMMAND $<TARGET_FILE:Qt6::lupdate>
    -recursive
    fem/include
    fem/src
    XLSXEditor/include
    XLSXEditor/src
    ProjectControlWidget/include
    ProjectControlWidget/src
    -ts ${TRANSLATION_TS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Update Qt translation files (.ts) for all modules"
)

# 编译 .ts -> .qm (use Qt-provided lrelease)
add_custom_target(compile_translations
    COMMAND $<TARGET_FILE:Qt6::lrelease> ${TRANSLATION_TS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Compile Qt translation files (.qm)"
)

# Ensure .ts are refreshed before compiling .qm
add_dependencies(compile_translations update_translations)

# 添加到 FemApp 编译依赖
add_dependencies(FemApp
    update_translations
    compile_translations
)


# Deploy Qt dependencies (integrated in toolchain configuration)
if(NOT FEMAPP_LINUX_BUILD_DEMO)
    deploy_qt_dependencies(FemApp)
endif()

if(WIN32)
    if(TARGET OpenXLSX)
        add_custom_command(TARGET FemApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:OpenXLSX>
                $<TARGET_FILE_DIR:FemApp>
            COMMENT "Copying OpenXLSX runtime to output directory"
        )
    endif()
    if(TARGET FEMConfig)
        add_custom_command(TARGET FemApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FEMConfig>
                $<TARGET_FILE_DIR:FemApp>
            COMMENT "Copying FEMConfig runtime to output directory"
        )
    endif()
    if(TARGET FileAssocLib)
        add_custom_command(TARGET FemApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:FileAssocLib>
                $<TARGET_FILE_DIR:FemApp>
            COMMENT "Copying FileAssocLib runtime to output directory"
        )
    endif()
endif()

# Move translation files to output directory after build
add_custom_command(TARGET FemApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:FemApp>/translations
    COMMAND ${CMAKE_COMMAND}
        -DSRC_DIR=${CMAKE_SOURCE_DIR}/translations
        -DDST_DIR=$<TARGET_FILE_DIR:FemApp>/translations
        -P ${CMAKE_SOURCE_DIR}/cmake/CopyQm.cmake
    COMMENT "Copying translation files to output directory"
)
