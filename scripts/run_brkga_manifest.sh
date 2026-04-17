#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Uso: $0 <arquivo_manifesto.txt> <tempo_limite_s>"
    exit 1
fi

MANIFEST=$1
TIME_LIMIT=$2
RESULTS_DIR="results/cvrp/brkga_runs/t${TIME_LIMIT}"

mkdir -p "$RESULTS_DIR"

while IFS= read -r line || [ -n "$line" ]; do
    if [[ -z "$line" ]] || [[ "$line" == \#* ]]; then
        continue
    fi

    INSTANCE_PATH=$(echo "$line" | awk '{print $1}')
    INSTANCE_NAME=$(basename "$INSTANCE_PATH" .vrp)
    
    if [ ! -f "$INSTANCE_PATH" ]; then
        continue
    fi

    LOG_FILE="${RESULTS_DIR}/${INSTANCE_NAME}_t${TIME_LIMIT}.log"
    ./build-linux/cvrp_brkga "$INSTANCE_PATH" "$TIME_LIMIT" > "$LOG_FILE"

done < "$MANIFEST"