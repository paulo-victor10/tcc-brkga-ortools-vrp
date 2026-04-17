#!/usr/bin/env bash
set -uo pipefail

MANIFEST="${1:-data/manifests/cvrp_experimental.txt}"
TIME_LIMIT="${2:-2}"

RESULTS_DIR="results/cvrp/runs/t${TIME_LIMIT}"
mkdir -p "$RESULTS_DIR"

missing_log="${RESULTS_DIR}/_missing_files.log"
failed_log="${RESULTS_DIR}/_failed_runs.log"

: > "$missing_log"
: > "$failed_log"

fail_count=0
run_count=0

while IFS= read -r line || [ -n "$line" ]; do
  line="${line//$'\r'/}"
  [[ -z "$line" ]] && continue
  [[ "$line" =~ ^# ]] && continue

  inst="$(echo "$line" | awk '{print $1}')"
  vehicles="$(echo "$line" | awk '{print $2}')"

  if [[ -z "${inst:-}" || -z "${vehicles:-}" ]]; then
    echo "Skipping invalid line: $line"
    continue
  fi

  if [[ ! -f "$inst" ]]; then
    echo "MISSING FILE: $inst"
    echo "$inst" >> "$missing_log"
    fail_count=$((fail_count+1))
    continue
  fi

  base="$(basename "$inst" .vrp)"
  echo "Running $base (vehicles=$vehicles, time_limit=${TIME_LIMIT}s)"
  run_count=$((run_count+1))

  log="${RESULTS_DIR}/${base}_t${TIME_LIMIT}.log"
  if ! ./build-linux/cvrp_vrplib "$inst" "$vehicles" "$TIME_LIMIT" >"$log" 2>&1; then
    echo "FAILED: $base"
    echo "$inst" >> "$failed_log"
    fail_count=$((fail_count+1))
    continue
  fi
done < "$MANIFEST"

echo "Done. Runs: $run_count. Failures: $fail_count."