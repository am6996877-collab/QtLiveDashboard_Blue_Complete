#!/usr/bin/env bash
set -euo pipefail
ENGINE_BIN="$1"
TRANS_BIN="$2"
BCM_BIN="$3"
DASHBOARD_BIN="$4"
if ! ip -o link show vcan0 | grep -qE '<[^>]*\bUP\b'; then
    echo 'Create and enable vcan0 before running the demo.' >&2
    exit 1
fi
PIDS=()
cleanup() {
    for pid in "${PIDS[@]}"; do kill "$pid" 2>/dev/null || true; done
    for pid in "${PIDS[@]}"; do wait "$pid" 2>/dev/null || true; done
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
"$ENGINE_BIN" --demo &
PIDS+=("$!")
"$TRANS_BIN" &
PIDS+=("$!")
"$BCM_BIN" &
PIDS+=("$!")
sleep 1
for pid in "${PIDS[@]}"; do
    if ! kill -0 "$pid" 2>/dev/null; then
        echo 'An ECU exited during startup; check its output.' >&2
        exit 1
    fi
done
"$DASHBOARD_BIN" &
DASHBOARD_PID=$!
PIDS+=("$DASHBOARD_PID")
wait "$DASHBOARD_PID"
