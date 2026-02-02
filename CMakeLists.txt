cmake_minimum_required(VERSION 3.10)

# Toolchain must be included BEFORE project()
# include(${CMAKE_SOURCE_DIR}/cmake/WindowsOfficialQt.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/LinuxCross.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/mingw64.cmake)


# Ensure CMAKE_PREFIX_PATH is set correctly for find_package
list(APPEND CMAKE_PREFIX_PATH "${QT_ROOT_DIR}")


project(FemApp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加 FEMConfig 库子项目
add_subdirectory(FEMConfig)
add_subdirectory(OpenXLSX)

# Add source files
set(SOURCES
        fem/src/main.cpp
        fem/src/FileAssociation.cpp
        fem/src/xlsx_proc.cpp
        fem/src/femapp.cpp
        fem/src/femapp_slots.cpp
)

set(HEADERS
        fem/include/
)

set(UI_HEADERS
        fem/include/ui_mainwindow.h
)
set(UI_FILES
        fem/src/mainwindow.ui
)


FILE(GLOB TRANSLATION_TS translations/*.ts)

# Create executable (removed WIN32 to show console for debugging cout)
add_executable(FemApp ${SOURCES})

# Include directories
target_include_directories(FemApp PRIVATE
        ${HEADERS}
        ${CMAKE_SOURCE_DIR}/FEMConfig/include
)

# Link libraries
target_link_libraries(FemApp PRIVATE
        Qt6::Core
        Qt6::Widgets
        FEMConfig
        OpenXLSX
        shlwapi
        shell32
)

# 生成 / 更新 .ts 文件
add_custom_target(update_translations
        COMMAND lupdate
        ${SOURCES} ${HEADERS} ${UI_FILES}
        -ts ${TRANSLATION_TS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Update Qt translation files (.ts)"
)

# 编译 .ts -> .qm
add_custom_target(compile_translations
        COMMAND lrelease ${TRANSLATION_TS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Compile Qt translation files (.qm)"
)

# 添加到 FemApp 编译依赖
add_dependencies(FemApp
        update_translations
        compile_translations
)


# Deploy Qt dependencies (integrated in toolchain configuration)
deploy_qt_dependencies(FemApp)

# Move translation files to output directory after build
add_custom_command(TARGET FemApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:FemApp>/translations
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/translations/*.qm
        $<TARGET_FILE_DIR:FemApp>/translations
        COMMENT "Copying translation files to output directory"
)
