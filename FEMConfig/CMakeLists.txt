cmake_minimum_required(VERSION 3.10)
project(FEMConfig)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(FEMCONFIG_BUILD_TEST "Build test app of FEMConfig" OFF)

# Add source files
set(SOURCES
  src/FEMConfig.cpp
)

set(HEADERS
  include/
)

# Create Lib
add_library(FEMConfig SHARED ${SOURCES})

# Include directories
target_include_directories(FEMConfig PUBLIC ${HEADERS})

# Export symbols from this target when building the DLL
target_compile_definitions(FEMConfig PRIVATE FEMCONFIG_EXPORTS)

# Link libraries (add as needed)
target_link_libraries(FEMConfig PRIVATE MiniXLSX)

if(FEMCONFIG_BUILD_TEST)
set(SOURCES_TEST
  test.cpp
)  

add_executable(FEMConfig_test ${SOURCES_TEST})
target_link_libraries(FEMConfig_test PRIVATE FEMConfig)
target_include_directories(FEMConfig_test PRIVATE ${HEADERS})
target_link_libraries(
FEMConfig_test PRIVATE MiniXLSX
)
endif(FEMCONFIG_BUILD_TEST)
