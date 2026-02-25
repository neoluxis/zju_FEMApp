cmake_minimum_required(VERSION 3.10)
project(MultiPrjWs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    src/multi_project_workspace.cpp
    src/multi_prj_ws_widget.cpp
    src/multiprjws.ui
    include/cc/neolux/fem/mpw/multi_project_workspace.h
    include/cc/neolux/fem/mpw/multi_prj_ws_widget.h
)

add_library(MultiPrjWs SHARED ${SOURCES})

target_include_directories(MultiPrjWs PUBLIC
    include
)

target_link_libraries(MultiPrjWs PUBLIC
    Qt6::Core
    Qt6::Widgets
)
