cmake_minimum_required(VERSION 3.10)
project(FileAssocLib)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
if(FEMAPP_LINUX_BUILD_DEMO)
  set(SOURCES
    src/FileAssociation_stub.cpp
  )
else()
  set(SOURCES
    src/FileAssociation.cpp
  )
endif()

set(HEADERS
  include/
)

# Create shared library
add_library(FileAssocLib SHARED ${SOURCES})

# Include directories
target_include_directories(FileAssocLib PUBLIC ${HEADERS})

# Link libraries (Windows specific)
if(WIN32)
  target_link_libraries(FileAssocLib PRIVATE shell32)
endif()
