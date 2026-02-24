# Linux cross-compilation toolchain configuration
# Complete self-contained configuration - no other includes needed
#
# Usage:
#   In CMakeLists.txt: include(${CMAKE_SOURCE_DIR}/cmake/LinuxCross.cmake)
#   Command line: cmake .. -G Ninja

message(STATUS "=== Linux Cross-Compilation to Windows ===")
message(STATUS "Using Linux cross-compilation configuration")

# ============================================================================
# Cross-compilation Toolchain Configuration
# ============================================================================
set(CROSS_PREFIX "/usr/x86_64-w64-mingw32")

if(NOT EXISTS "${CROSS_PREFIX}")
    message(FATAL_ERROR "Cross-compilation toolchain not found at ${CROSS_PREFIX}")
endif()

# Qt6 path for cross-compilation
set(QT_ROOT_DIR "${CROSS_PREFIX}")
set(CMAKE_PREFIX_PATH "${CROSS_PREFIX}")
set(QT_BIN_DIR "${CROSS_PREFIX}/lib/qt6/bin")
set(QT_PLUGINS_DIR "${CROSS_PREFIX}/lib/qt6/plugins")

# MinGW runtime path
set(MINGW_BIN_DIR "${CROSS_PREFIX}/bin")

# ============================================================================
# Compiler and Linker Flags for Proper Symbol Handling
# ============================================================================
# For cross-compilation to Windows, we need to handle libstdc++ symbols carefully.
#
# Problem: Some codecvt (character conversion) symbols from libstdc++ are not
# available in Wine's shared libstdc++-6.dll. We need to embed libstdc++ into DLLs.
#
# However, linking with -static-libgcc -static-libstdc++ causes duplicate symbol
# errors (_Unwind_Resume) when multiple DLLs are linked into the final executable.
#
# Solution: Use -Wl,--allow-multiple-definition to permit duplicate symbols.
# This is safe because both copies are identical and only the first is used.

# For shared libraries, statically link libstdc++ but allow duplicate symbols
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++ -Wl,--allow-multiple-definition")

# For executables, use static linking to reduce runtime dependencies
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

# Add flag to ensure proper C++ ABI compatibility and symbol visibility
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

message(STATUS "Cross-compilation prefix: ${CROSS_PREFIX}")
message(STATUS "Qt root: ${QT_ROOT_DIR}")
message(STATUS "Qt bin: ${QT_BIN_DIR}")
message(STATUS "Qt plugins: ${QT_PLUGINS_DIR}")
message(STATUS "MinGW bin: ${MINGW_BIN_DIR}")

# =========================================================================
# Deployment Configuration (integrated)
# ============================================================================

# This function will be called after target is created
function(deploy_qt_dependencies target)
    set(DEPLOY_DIR $<TARGET_FILE_DIR:${target}>)

    # For cross-compilation, manually copy required DLLs
    # (windeployqt doesn't work in cross-compilation environment)


    # Copy Qt DLLs
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_BIN_DIR}/Qt6Core.dll"
            "${QT_BIN_DIR}/Qt6Gui.dll"
            "${QT_BIN_DIR}/Qt6Widgets.dll"
            ${DEPLOY_DIR}
        COMMENT "Copying Qt DLLs"
    )

    # Copy Qt platform plugin
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR}/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QT_PLUGINS_DIR}/platforms/qwindows.dll"
            ${DEPLOY_DIR}/platforms/
        COMMENT "Copying Qt platform plugin"
    )

    # Copy MinGW runtime DLLs
    file(GLOB MINGW_DLLS
        "${MINGW_BIN_DIR}/libgcc_*.dll"
        "${MINGW_BIN_DIR}/libstdc++*.dll"
        "${MINGW_BIN_DIR}/libwinpthread*.dll"
    )

    if(MINGW_DLLS)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MINGW_DLLS} ${DEPLOY_DIR}
            COMMENT "Copying MinGW runtime DLLs"
        )
    endif()

    # Create qt.conf
    file(WRITE "${CMAKE_BINARY_DIR}/qt.conf" "[Paths]\nPlugins = .\n")
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/qt.conf"
            ${DEPLOY_DIR}/qt.conf
        COMMENT "Creating qt.conf"
    )

    message(STATUS "Deployment configured for target: ${target}")
endfunction()

message(STATUS "=== Toolchain configuration complete ===")
