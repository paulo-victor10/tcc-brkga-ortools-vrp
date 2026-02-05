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
    in_csv = sys.argv[1]
    out_csv = sys.argv[2]
    instances_root = sys.argv[3]

    with open(in_csv, "r", encoding="utf-8") as f:
        rows = list(csv.DictReader(f))

    missing = 0
    no_cost = 0

    for r in rows:
        instance = r["instance"]
        sol_path = find_sol_file(instances_root, instance)
        bks = read_bks_cost(sol_path) if sol_path else None

        if sol_path is None:
            missing += 1
        elif bks is None:
            no_cost += 1

        r["bks_cost"] = "" if bks is None else str(bks)

        try:
            final_cost = int(r["total_distance"])
            best_cost = int(r.get("best_cost", final_cost))
        except Exception:
            r["gap_final_pct"] = ""
            r["gap_best_pct"] = ""
            continue

        if bks and bks > 0:
            r["gap_final_pct"] = f"{((final_cost - bks) / bks) * 100.0:.2f}"
            r["gap_best_pct"] = f"{((best_cost - bks) / bks) * 100.0:.2f}"
        else:
            r["gap_final_pct"] = ""
            r["gap_best_pct"] = ""

    fieldnames = list(rows[0].keys())
    for c in ["bks_cost", "gap_final_pct", "gap_best_pct"]:
        if c not in fieldnames:
            fieldnames.append(c)

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
