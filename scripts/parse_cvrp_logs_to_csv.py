import csv
import os
import re
import sys

RE_INSTANCE = re.compile(
    r"Instance:\s*(\d+)\s*nodes,\s*capacity\s*(\d+),\s*vehicles\s*(\d+),\s*depot\s*(\d+)"
)
RE_TOTAL_DISTANCE = re.compile(r"Total distance:\s*(\d+)")
RE_SOLVED_MS = re.compile(r"Solved in\s*(\d+)\s*ms")

def parse_time_limit_from_filename(filename: str) -> int:
    m = re.search(r"_t(\d+)\.log$", filename)
    return int(m.group(1)) if m else -1

def parse_log(path: str) -> dict:
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        text = f.read()

    m1 = RE_INSTANCE.search(text)
    m2 = RE_TOTAL_DISTANCE.search(text)
    m3 = RE_SOLVED_MS.search(text)

    if not (m1 and m2 and m3):
        raise ValueError(f"Could not parse required fields from log: {path}")

    n_nodes = int(m1.group(1))
    capacity = int(m1.group(2))
    vehicles = int(m1.group(3))
    depot = int(m1.group(4))
    total_distance = int(m2.group(1))
    solved_ms = int(m3.group(1))

    filename = os.path.basename(path)
    time_limit_s = parse_time_limit_from_filename(filename)
    instance = filename.replace(f"_t{time_limit_s}.log", "") if time_limit_s >= 0 else os.path.splitext(filename)[0]

    return {
        "instance": instance,
        "n_nodes": n_nodes,
        "capacity": capacity,
        "vehicles": vehicles,
        "depot": depot,
        "time_limit_s": time_limit_s,
        "total_distance": total_distance,
        "solved_ms": solved_ms,
        "log_file": filename,
    }

def main() -> int:
    logs_dir = sys.argv[1] if len(sys.argv) >= 2 else "results/cvrp/runs/t2"
    out_csv = sys.argv[2] if len(sys.argv) >= 3 else "results/cvrp/summary_t2.csv"

    if not os.path.isdir(logs_dir):
        print(f"ERROR: logs_dir not found: {logs_dir}", file=sys.stderr)
        return 2

    rows = []
    for name in sorted(os.listdir(logs_dir)):
        if not name.endswith(".log"):
            continue
        path = os.path.join(logs_dir, name)
        try:
            rows.append(parse_log(path))
        except Exception as e:
            print(f"WARNING: {e}", file=sys.stderr)

    if not rows:
        print("ERROR: no parsed rows; check logs directory.", file=sys.stderr)
        return 3

    fieldnames = ["instance", "n_nodes", "capacity", "vehicles", "depot", "time_limit_s", "total_distance", "solved_ms", "log_file"]
    os.makedirs(os.path.dirname(out_csv), exist_ok=True)
    with open(out_csv, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        for r in rows:
            w.writerow(r)

    print(f"Wrote {len(rows)} rows to {out_csv}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
