# 常用的 build type
set(AVAILABLE_BUILD_TYPES
    "Debug"
    "Release"
    "RelWithDebInfo"
    "MinSizeRel"
)

# 如果没有指定 build type，则设置为 Release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified. Defaulting to Release.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
# 检查指定的 build type 是否有效
if(NOT CMAKE_BUILD_TYPE IN_LIST AVAILABLE_BUILD_TYPES)
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}. Available types are: ${AVAILABLE_BUILD_TYPES}")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
# 根据 build type 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for Debug build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring for Release build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "Configuring for RelWithDebInfo build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g
    -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    message(STATUS "Configuring for MinSizeRel build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -DNDEBUG")
endif()
