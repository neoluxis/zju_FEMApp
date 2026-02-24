# Windows Qt6 from official D:/Qt installation + MinGW64 toolchain
# Complete self-contained configuration - no other includes needed
#
# Usage:
#   In CMakeLists.txt: include(${CMAKE_SOURCE_DIR}/cmake/WindowsOfficialQt.cmake)
#   Command line: cmake .. -G Ninja -DQT_ROOT_DIR="D:/Qt/6.10.1/mingw_64" -DMINGW_ROOT="D:/Scoop/apps/msys2/current/mingw64"

message(STATUS "=== Windows Official Qt6 (D:/Qt) + MinGW64 Toolchain ===")

# ============================================================================
# Qt6 Configuration from D:/Qt
# ============================================================================
set(QT_ROOT_DIR D:/Qt/6.10.1/mingw_64)
if(NOT DEFINED QT_ROOT_DIR)
    message(FATAL_ERROR "QT_ROOT_DIR is not defined. Please specify: cmake -DQT_ROOT_DIR=<path to D:/Qt/6.x.x/mingw_64>")
endif()

if(NOT EXISTS "${QT_ROOT_DIR}")
    message(FATAL_ERROR "Qt6 path does not exist: ${QT_ROOT_DIR}")
endif()

set(CMAKE_PREFIX_PATH "${QT_ROOT_DIR}")
set(QT_BIN_DIR "${QT_ROOT_DIR}/bin")
set(QT_PLUGINS_DIR "${QT_ROOT_DIR}/plugins")

message(STATUS "Qt source: Official (D:/Qt)")
message(STATUS "Qt root: ${QT_ROOT_DIR}")
message(STATUS "Qt bin: ${QT_BIN_DIR}")
message(STATUS "Qt plugins: ${QT_PLUGINS_DIR}")

# ============================================================================
# MinGW64 Toolchain Configuration (use system gcc from scoop)
# ============================================================================
# MINGW_ROOT is optional - used only for static linking flags if needed
if(DEFINED MINGW_ROOT)
    if(NOT EXISTS "${MINGW_ROOT}")
        message(WARNING "MinGW path does not exist: ${MINGW_ROOT}")
    else()
        set(MINGW_BIN_DIR "${MINGW_ROOT}/bin")
        message(STATUS "MinGW root: ${MINGW_ROOT}")
        message(STATUS "MinGW bin: ${MINGW_BIN_DIR}")
    endif()
endif()


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
