#!/usr/bin/env bash
set -uo pipefail

MANIFEST="${1:-data/manifests/cvrp_experimental.txt}"
TIME_LIMIT="${2:-2}"

mkdir -p results/cvrp
missing_log="results/cvrp/_missing_files_t${TIME_LIMIT}.log"
failed_log="results/cvrp/_failed_runs_t${TIME_LIMIT}.log"

: > "$missing_log"
: > "$failed_log"

fail_count=0
run_count=0

while IFS= read -r line; do
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

  if ! ./build-linux/cvrp_vrplib "$inst" "$vehicles" "$TIME_LIMIT" \
      | tee "results/cvrp/${base}_t${TIME_LIMIT}.log"; then
    echo "FAILED: $base"
    echo "$inst" >> "$failed_log"
    fail_count=$((fail_count+1))
    continue
  fi
done < "$MANIFEST"

echo "Done. Runs: $run_count. Failures: $fail_count."