#!/usr/bin/env bash
set -euo pipefail

for command_name in cmake c++; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "[setup_env.sh, ERROR] Required command not found: $command_name"
        exit 1
    fi
done

if ! printf '#include <zmq.hpp>\n' | c++ -x c++ -E - >/dev/null 2>&1; then
    echo "[setup_env.sh, ERROR] cppzmq header zmq.hpp was not found."
    exit 1
fi

if ! ldconfig -p 2>/dev/null | grep 'libzmq' >/dev/null; then
    echo "[setup_env.sh, ERROR] libzmq was not found in the linker cache."
    exit 1
fi

echo "[setup_env.sh] DQM gateway prerequisites are available."

