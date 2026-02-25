cmake_minimum_required(VERSION 3.10)
project(ProjectControlWidget)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    src/project_control_widget.cpp
    src/projectcontrolwidget.ui
    include/cc/neolux/projectcontrol/project_control_widget.h
)

set(HEADERS
    include/
)

add_library(ProjectControlWidget SHARED ${SOURCES})

target_include_directories(ProjectControlWidget PUBLIC
    ${HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(ProjectControlWidget PUBLIC
    Qt6::Core
    Qt6::Widgets
)
