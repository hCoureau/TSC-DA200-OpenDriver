#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
port=${DA200_AIRPRINT_TEST_PORT:-18000}
spool=$(mktemp -d)
log=$(mktemp)

cleanup() {
  if [ -n "${server_pid:-}" ]; then
    kill "$server_pid" 2>/dev/null || true
  fi
  rm -rf "$spool" "$log"
}
trap cleanup EXIT HUP INT TERM

"$project_dir/build/tsc-da200-printer-app" server \
  -o "server-port=$port" \
  -o "device-uri=socket://127.0.0.1:9100" \
  -o "spool-directory=$spool" >"$log" 2>&1 &
server_pid=$!

for _ in 1 2 3 4 5; do
  if ipptool -q "ipp://localhost:$port/ipp/print" \
      "$project_dir/tests/airprint-attributes.test" >/dev/null 2>&1; then
    exit 0
  fi
  sleep 1
done

cat "$log" >&2
exit 1
