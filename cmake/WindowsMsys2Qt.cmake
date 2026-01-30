# Windows Qt6 from MSYS2 D:/Scoop/apps/msys2 installation + MinGW64 toolchain
# Complete self-contained configuration - no other includes needed
# 
# Usage:
#   In CMakeLists.txt: include(${CMAKE_SOURCE_DIR}/cmake/WindowsMsys2Qt.cmake)
#   Command line: cmake .. -G Ninja -DQT_ROOT_DIR="D:/Scoop/apps/msys2/current/mingw64" -DMINGW_ROOT="D:/Scoop/apps/msys2/current/mingw64"

message(STATUS "=== Windows MSYS2 Qt6 (D:/Scoop/apps/msys2) + MinGW64 Toolchain ===")

if(NOT DEFINED QT_ROOT_DIR)
    message(FATAL_ERROR "QT_ROOT_DIR is not defined. Please specify: cmake -DQT_ROOT_DIR=<path to D:/Scoop/apps/msys2/current/mingw64>")
endif()

if(NOT EXISTS "${QT_ROOT_DIR}")
    message(FATAL_ERROR "Qt6 path does not exist: ${QT_ROOT_DIR}")
endif()

set(CMAKE_PREFIX_PATH "${QT_ROOT_DIR}")
set(QT_BIN_DIR "${QT_ROOT_DIR}/bin")
set(QT_PLUGINS_DIR "${QT_ROOT_DIR}/share/qt6/plugins")

message(STATUS "Qt source: MSYS2 (D:/Scoop/apps/msys2)")
message(STATUS "Qt root: ${QT_ROOT_DIR}")
message(STATUS "Qt bin: ${QT_BIN_DIR}")
message(STATUS "Qt plugins: ${QT_PLUGINS_DIR}")

if(NOT DEFINED MINGW_ROOT)
    message(FATAL_ERROR "MINGW_ROOT is not defined. Please specify: cmake -DMINGW_ROOT=<path>")
endif()

if(NOT EXISTS "${MINGW_ROOT}")
    message(FATAL_ERROR "MinGW path does not exist: ${MINGW_ROOT}")
endif()

# ============================================================================
# MinGW64 Toolchain Configuration (use system gcc from scoop)
# ============================================================================
if(DEFINED MINGW_ROOT)
    if(NOT EXISTS "${MINGW_ROOT}")
        message(WARNING "MinGW path does not exist: ${MINGW_ROOT}")
    else()
        set(MINGW_BIN_DIR "${MINGW_ROOT}/bin")
        message(STATUS "MinGW root: ${MINGW_ROOT}")
        message(STATUS "MinGW bin: ${MINGW_BIN_DIR}")
    endif()
endif()

# Static linking for MinGW runtime
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")

message(STATUS "Using system GCC from PATH (scoop installed)")



# This function will be called after target is created
function(deploy_qt_dependencies target)
    set(DEPLOY_DIR $<TARGET_FILE_DIR:${target}>)
    
    # Find windeployqt executable
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
    
    if(NOT WINDEPLOYQT_EXECUTABLE)
        message(WARNING "windeployqt not found. Qt dependencies may not be deployed correctly.")
    else()
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
        
        # Run windeployqt to automatically copy all Qt dependencies
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                "$<TARGET_FILE:${target}>"
            COMMENT "Deploying Qt dependencies using windeployqt"
            VERBATIM
        )
    endif()
    
    message(STATUS "Deployment configured for target: ${target}")
endfunction()

