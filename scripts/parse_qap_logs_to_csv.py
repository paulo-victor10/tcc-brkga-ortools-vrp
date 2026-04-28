import os
import re
import csv

RESULTS_DIR = "results/qap/runs"
OUTPUT_CSV = "results/qap/qap_summary_final.csv"

def parse_logs():
    manifest_path = "data/manifests/qap_final_experiment.txt"
    if not os.path.exists(manifest_path):
        print("Manifesto não encontrado.")
        return

    with open(manifest_path, 'r') as f:
        instances = [line.strip() for line in f if line.strip()]

    data = []

    for inst in instances:
        row = {"Instancia": inst, "N": re.search(r'\d+', inst).group()}
        
        # --- Lendo OR-Tools ---
        ortools_log = os.path.join(RESULTS_DIR, f"ortools/{inst}.log")
        row["ORT_Custo"] = "TIMEOUT/FALHA"
        row["ORT_Tempo(s)"] = "300"
        if os.path.exists(ortools_log):
            with open(ortools_log, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                # Tenta pegar do nosso print customizado
                custo_match = re.search(r'Custo \(Distancia Total\): (\d+)', content)
                tempo_match = re.search(r'Tempo decorrido: ([\d\.]+)s', content)
                if custo_match:
                    row["ORT_Custo"] = custo_match.group(1)
                if tempo_match:
                    row["ORT_Tempo(s)"] = tempo_match.group(1)

        # --- Lendo BRKGA ---
        brkga_log = os.path.join(RESULTS_DIR, f"brkga/{inst}.log")
        row["BRKGA_Custo"] = "FALHA"
        row["BRKGA_Geracoes"] = "0"
        row["BRKGA_Tempo(s)"] = "300"
        if os.path.exists(brkga_log):
            with open(brkga_log, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                custo_match = re.search(r'Custo Final: (\d+)', content)
                ger_match = re.search(r'Geracoes: (\d+)', content)
                
                if custo_match:
                    row["BRKGA_Custo"] = custo_match.group(1)
                if ger_match:
                    row["BRKGA_Geracoes"] = ger_match.group(1)

        data.append(row)

    # --- Salvando em CSV ---
    with open(OUTPUT_CSV, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=["Instancia", "N", "ORT_Custo", "ORT_Tempo(s)", "BRKGA_Custo", "BRKGA_Geracoes", "BRKGA_Tempo(s)"])
        writer.writeheader()
        writer.writerows(data)
    
    print(f"✅ Sucesso! Resumo gerado em: {OUTPUT_CSV}")

if __name__ == '__main__':
    parse_logs()
