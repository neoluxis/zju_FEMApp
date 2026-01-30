cmake_minimum_required(VERSION 3.10)
project(FEMConfig)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
  src/FEMConfig.cpp
)

set(HEADERS
  include/
  
)

# Create Lib
add_library(FEMConfig ${SOURCES})

# Include directories
target_include_directories(FEMConfig PRIVATE ${HEADERS})

# Link libraries (add as needed)
target_link_libraries(FEMConfig 
PRIVATE 
OpenXLSX)