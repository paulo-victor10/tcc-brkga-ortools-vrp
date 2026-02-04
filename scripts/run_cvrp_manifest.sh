#!/usr/bin/env bash
set -euo pipefail

MANIFEST="${1:-data/manifests/cvrp_test.txt}"
TIME_LIMIT="${2:-2}"

mkdir -p results/cvrp

while read -r inst vehicles; do
  [ -z "${inst:-}" ] && continue
  base="$(basename "$inst" .vrp)"
  echo "Running $base (vehicles=$vehicles, time_limit=${TIME_LIMIT}s)"
  ./build-linux/cvrp_vrplib "$inst" "$vehicles" "$TIME_LIMIT" | tee "results/cvrp/${base}_t${TIME_LIMIT}.log"
done < "$MANIFEST"
