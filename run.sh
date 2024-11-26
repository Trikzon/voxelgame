#!/bin/bash

BUILD_MODE="Debug"
BUILD_DIR="build"

usage() {
    echo "Usage: $0 [debug|release]"
    exit 1
}

# Parse arguments
if [ "$#" -gt 1 ]; then
    usage
elif [ "$#" -eq 1 ]; then
    case "$1" in
        debug|Debug)
            BUILD_MODE="Debug"
            ;;
        release|Release)
            BUILD_MODE="Release"
            ;;
        *)
            echo "Invalid argument: $1"
            usage
            ;;
    esac
fi

echo "Selected Build Mode: $BUILD_MODE"

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cmake -DCMAKE_BUILD_TYPE="$BUILD_MODE" -B "$BUILD_DIR" || exit 1;
cmake --build "$BUILD_DIR" --target sandbox || exit 1;

clear
"$BUILD_DIR/games/sandbox/sandbox"
