#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR=$(dirname "$(realpath "$0")")
PROJECT_ROOT=$(realpath "$SCRIPT_DIR/..")
EXECUTABLE="$PROJECT_ROOT/build/bin/dqm_gateway"
CONFIG_PATH="$PROJECT_ROOT/config/config.json"
ADDRESS=""
PORT=""
THREADS=""
PLUGIN_DIR=""

show_help() {
    cat <<EOF
Usage: ./scripts/run.sh [OPTIONS]

Options:
  --config PATH       Gateway configuration (default: config/config.json)
  --address ADDRESS   Override the configured listen address
  --port PORT         Override the configured HTTP port
  --threads COUNT     Override the configured HTTP worker count
  --plugin-dir PATH   Override the build's plugin directory
  -h, --help          Display this help message
EOF
}

require_value() {
    if [[ $# -lt 2 || -z "${2:-}" || "${2:-}" == -* ]]; then
        echo "[run.sh, ERROR] $1 requires a value" >&2
        exit 2
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --config) require_value "$@"; CONFIG_PATH=$2; shift 2 ;;
        --address) require_value "$@"; ADDRESS=$2; shift 2 ;;
        --port) require_value "$@"; PORT=$2; shift 2 ;;
        --threads) require_value "$@"; THREADS=$2; shift 2 ;;
        --plugin-dir) require_value "$@"; PLUGIN_DIR=$2; shift 2 ;;
        -h|--help) show_help; exit 0 ;;
        *) echo "[run.sh, ERROR] Unknown option: $1" >&2; show_help; exit 2 ;;
    esac
done

if [[ ! -x "$EXECUTABLE" ]]; then
    echo "[run.sh, ERROR] Build the gateway first with ./scripts/build.sh" >&2
    exit 1
fi

COMMAND=("$EXECUTABLE" --config "$(realpath "$CONFIG_PATH")")
[[ -n "$ADDRESS" ]] && COMMAND+=(--address "$ADDRESS")
[[ -n "$PORT" ]] && COMMAND+=(--port "$PORT")
[[ -n "$THREADS" ]] && COMMAND+=(--threads "$THREADS")
[[ -n "$PLUGIN_DIR" ]] && COMMAND+=(--plugin-dir "$(realpath "$PLUGIN_DIR")")

cd "$PROJECT_ROOT"
exec "${COMMAND[@]}"
