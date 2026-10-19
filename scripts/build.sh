#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")
BUILD_DIR="$PROJECT_ROOT/build"
OVERWRITE=false
JOBS=""
BUILD_TYPE="Release"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -o|--overwrite) OVERWRITE=true; shift ;;
        -j|--jobs) JOBS="${2:?--jobs requires a value}"; shift 2 ;;
        --build-type) BUILD_TYPE="${2:?--build-type requires a value}"; shift 2 ;;
        -h|--help)
            echo "Usage: ./scripts/build.sh [--overwrite] [--jobs N] [--build-type TYPE]"
            exit 0
            ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [[ "$OVERWRITE" == true ]]; then
    cmake -E remove_directory "$BUILD_DIR"
fi

cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
if [[ -n "$JOBS" ]]; then
    cmake --build "$BUILD_DIR" --parallel "$JOBS"
else
    cmake --build "$BUILD_DIR" --parallel
fi
