#!/bin/bash
# @author: neolux
# @date: 2026-02-27
# @brief: Auto setup, build and package script for FemApp on Windows
#         using CMake and Ninja with MinGW-w64 toolchain

# function to parse cmd args
function parse_args() {
    local arg key value

    for arg in "$@"; do
        case "$arg" in
            --no-*=*)
                echo "Invalid argument: $arg" >&2
                exit 1
                ;;
            --no-*)
                key="${arg#--no-}"
                ;;
            --*=*)
                key="${arg#--}"
                key="${key%%=*}"
                value="${arg#*=}"
                ;;
            --*)
                key="${arg#--}"
                ;;
            *)
                echo "Unknown argument: $arg" >&2
                exit 1
                ;;
        esac

        key="${key//-/_}"

        if [[ ! "$key" =~ ^[a-zA-Z_][a-zA-Z0-9_]*$ ]]; then
            echo "Invalid variable name: $key" >&2
            exit 1
        fi

        if [[ "$arg" == --no-* ]]; then
            printf -v "$key" '%s' false
        elif [[ "$arg" == --*=* ]]; then
            printf -v "$key" '%s' "$value"
        else
            printf -v "$key" '%s' true
        fi
    done
}

# colored log functions
function log() {
    local timestamp
    timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    printf '[%s] %b\n' "$timestamp" "$*"
}

function log_debug() {
    log "\033[36m[DEBUG]\033[0m $*"
}
function log_info() {
    log "\033[32m[INFO]\033[0m $*"
}
function log_warning() {
    log "\033[33m[WARNING]\033[0m $*"
}
function log_error() {
    log "\033[31m[ERROR]\033[0m $*"
}

function help() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --help                      Show this help message and exit"
    echo "  --build-dir=DIR             Specify the build directory (default: build; env: BUILD_DIR)"
    echo "  --pack-dir=DIR              Specify the packaging directory (default: package; env: PACK_DIR)"
    echo "  --clean[-build, -package, -zip]   Clean the directories and exit"
    echo "  --no-clean                  Do not clean the build and packaging directories before building"
    echo "  --setup                     Setup the build environment and exit"
    echo "  --build                Build the project and exit"
    echo "  --package              Package the project and exit"
}

function proc_setup() {
    if [[ -f "$BUILD_DIR/.setup_done" ]]; then
        log_info "Setup already done: CMake configuration exists in $BUILD_DIR"
        return
    fi
    mkdir -pv $BUILD_DIR
    cmake -B $BUILD_DIR -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw64.cmake -DFEMAPP_ADD_WIN32=ON -DCMAKE_BUILD_TYPE=Release
    echo "true" > $BUILD_DIR/.setup_done
    log_info "Setup completed: CMake configuration generated in $BUILD_DIR"
}

function proc_build() {
    if [[ -f "$BUILD_DIR/.build_done" ]]; then
        log_info "Build already done: Executable exists in $BUILD_DIR"
        return
    fi

    if [[ ! "$clean" == false ]]; then
        proc_clean
    fi

    if [[ ! -f "$BUILD_DIR/.setup_done" ]]; then
        proc_setup
    fi

    cmake --build $BUILD_DIR -j
    echo "true" > $BUILD_DIR/.build_done
    log_info "Build completed: Executable generated in $BUILD_DIR"
}

function proc_package() {
    clean=false # do not clean by default when packaging, as it may be time-consuming to rebuild everything
    if [[ ! -f "$BUILD_DIR/.build_done" ]]; then
        proc_build
    fi

    mkdir -pv $PACK_DIR/translations

    cp -r $BUILD_DIR/FemApp.exe $PACK_DIR/
    cp -r /usr/x86_64-w64-mingw32/lib/qt6/plugins/* $PACK_DIR
    cp -r translations/*.qm $PACK_DIR/translations

    env DLLPATH=/usr/x86_64-w64-mingw32/bin:/usr/x86_64-w64-mingw32/lib:$BUILD_DIR \
    x86_64-w64-mingw32-dllcopy $PACK_DIR/FemApp.exe ./dll.txt -r

    cmthash=$(git rev-parse --short HEAD 2>/dev/null)
    if [[ -n "$cmthash" ]]; then
        fname="femapp_package_v$(date +%Y%m%d)_$cmthash.zip"
    else
        fname="femapp_package_v$(date +%Y%m%d).zip"
    fi
    if [[ -f "$fname" ]]; then
        log_warning "Package file $fname already exists and will be overwritten."
        rm -f "$fname"
    fi

    if command -v pv >/dev/null 2>&1; then
        file_count=$(find "$PACK_DIR" -type f | wc -l)
        if [[ "$file_count" -eq 0 ]]; then
            log_warning "Nothing to package in $PACK_DIR"
        else
            zip -r -D "$fname" "$PACK_DIR" | pv -l -i 0.2 -s "$file_count" -N "zip" -F '%N %p <%{transferred}/'"$file_count"'> %t %e' >/dev/null
            zip_status=${PIPESTATUS[0]}
            if [[ "$zip_status" -ne 0 ]]; then
                log_error "zip failed with status $zip_status"
                return "$zip_status"
            fi
        fi
    else
        log_warning "pv not found, packaging without progress bar"
        zip -rq "$fname" "$PACK_DIR"/*
    fi

    log_info "Packaging completed: Package created as $fname"
}

function proc_clean() {
    rm -rfv "$BUILD_DIR" "$PACK_DIR" | wc -l | awk '{print "Removed: " $1 " items"}'
    log_info "Cleaned build and package directories: $BUILD_DIR, $PACK_DIR"
}

# ----------

parse_args "$@"

# --help
if [[ "$help" == true ]]; then
    help
    exit 0
fi

BUILD_DIR="${build_dir:-${BUILD_DIR:-build}}"
PACK_DIR="${pack_dir:-${PACK_DIR:-package}}"

# clean build or package directories
if [[ "$clean_build" == true ]]; then
    rm -rfv "$BUILD_DIR" | wc -l | awk '{print "Removed: " $1 " items"}'
    log_info "Cleaned build directory: $BUILD_DIR"
    exit 0
fi
if [[ "$clean_package" == true ]]; then
    rm -rfv "$PACK_DIR" | wc -l | awk '{print "Removed: " $1 " items"}'
    log_info "Cleaned package directory: $PACK_DIR"
    exit 0
fi
if [[ "$clean_zip" == true ]]; then
    find . -maxdepth 1 -type f -name "femapp_package_v*.zip" -print0 | xargs -0 rm -fv | wc -l | awk '{print "Removed: " $1 " items"}'
    log_info "Cleaned zip packages in current directory"
    exit 0
fi

# --clean and --no-clean
if [[ "$clean" == true ]]; then
    proc_clean
fi

if [[ "$clean" == true && "$setup" == "" && "$build" == "" && "$package" == "" ]]; then
    exit 0
fi

# --package: this will skip setup and build steps, and directly package the existing build output, which is useful for quickly creating packages without rebuilding everything
if [[ "$package" == true ]]; then
    proc_package
    exit 0
fi

# --build
if [[ "$build" == true ]]; then
    proc_build
    exit 0
fi

proc_setup

if [[ "$setup" == true  || "$build" == false ]]; then
    exit 0
fi

proc_build

if [[ "$build" == true || "$package" == false ]]; then
    exit 0
fi

proc_package
