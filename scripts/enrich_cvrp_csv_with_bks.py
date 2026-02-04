import csv
import os
import re
import sys

RE_COST = re.compile(r"\bCost\s+(\d+)\b")

def find_sol_file(instances_root: str, instance_name: str) -> str | None:
    target = f"{instance_name}.sol"
    for root, _, files in os.walk(instances_root):
        if target in files:
            return os.path.join(root, target)
    return None

def read_bks_cost(sol_path: str) -> int | None:
    try:
        with open(sol_path, "r", encoding="utf-8", errors="replace") as f:
            text = f.read()
        m = RE_COST.search(text)
        return int(m.group(1)) if m else None
    except Exception:
        return None

def main() -> int:
    in_csv = sys.argv[1] if len(sys.argv) >= 2 else "results/cvrp/summary_t2.csv"
    out_csv = sys.argv[2] if len(sys.argv) >= 3 else "results/cvrp/summary_t2_with_bks.csv"
    instances_root = sys.argv[3] if len(sys.argv) >= 4 else "data/instances/cvrp"

    if not os.path.isfile(in_csv):
        print(f"ERROR: input CSV not found: {in_csv}", file=sys.stderr)
        return 2
    if not os.path.isdir(instances_root):
        print(f"ERROR: instances_root not found: {instances_root}", file=sys.stderr)
        return 3

    with open(in_csv, "r", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    if not rows:
        print("ERROR: input CSV has no rows.", file=sys.stderr)
        return 4

    missing = 0
    no_cost = 0

    for r in rows:
        instance = r.get("instance", "").strip()
        bks = None
        sol_path = find_sol_file(instances_root, instance)
        if sol_path:
            bks = read_bks_cost(sol_path)
            if bks is None:
                no_cost += 1
        else:
            missing += 1

        r["bks_cost"] = "" if bks is None else str(bks)

        gap = ""
        try:
            td = int(r.get("total_distance", ""))
            if bks is not None and bks > 0:
                gap = f"{((td - bks) / bks) * 100.0:.2f}"
        except Exception:
            gap = ""
        r["gap_pct"] = gap

    fieldnames = list(rows[0].keys())
    if "bks_cost" not in fieldnames:
        fieldnames.append("bks_cost")
    if "gap_pct" not in fieldnames:
        fieldnames.append("gap_pct")

    os.makedirs(os.path.dirname(out_csv), exist_ok=True)
    with open(out_csv, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        for r in rows:
            w.writerow(r)

    print(f"Wrote {len(rows)} rows to {out_csv}")
    print(f"Missing .sol files: {missing}")
    print(f".sol files without Cost: {no_cost}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
