cmake_minimum_required(VERSION 3.10)
project(Recent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

set(SOURCES
    src/recent_project_history.cpp
    include/cc/neolux/fem/recent/recent_project_history.h
)

add_library(Recent SHARED ${SOURCES})

target_include_directories(Recent PUBLIC
    include
)

target_link_libraries(Recent PUBLIC
    Qt6::Core
)
