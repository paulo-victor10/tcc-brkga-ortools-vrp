import csv
import os

INPUT_CSV = "results/qap/qap_summary_final.csv"
OUTPUT_CSV = "results/qap/qap_summary_final_enriched.csv"
SLN_DIR = "data/instances/qap"

# Dicionário de backup com as BKS (Best Known Solutions) exatas da QAPLIB
# Isso garante o funcionamento mesmo se você não tiver os arquivos .sln extraídos.
BKS_FALLBACK = {
    "nug12.dat": 578, "tai12a.dat": 224416, "chr12a.dat": 9552, "rou12.dat": 235528,
    "nug15.dat": 1150, "tai15a.dat": 388214, "chr15a.dat": 9896, "rou15.dat": 354210,
    "nug17.dat": 1732, "nug20.dat": 2570, "tai20a.dat": 703482, "chr20a.dat": 2192,
    "rou20.dat": 725522, "nug22.dat": 3596, "nug24.dat": 3488, "nug25.dat": 3744,
    "tai25a.dat": 1167256
}

def get_bks(instance):
    sln_file = os.path.join(SLN_DIR, instance.replace(".dat", ".sln"))
    # Se o arquivo .sln existir, lemos a BKS direto dele
    if os.path.exists(sln_file):
        try:
            with open(sln_file, 'r') as f:
                parts = f.read().split()
                if len(parts) >= 2:
                    return float(parts[1])
        except:
            pass
    # Se não existir ou falhar, usamos o backup oficial
    return BKS_FALLBACK.get(instance, None)

def calc_gap(cost, bks):
    try:
        c = float(cost)
        b = float(bks)
        # Fórmula do Gap Relativo: ((Custo_Encontrado - BKS) / BKS) * 100
        return round(((c - b) / b) * 100, 2)
    except (ValueError, TypeError):
        return "N/A"

def determine_winner(ort, brkga):
    try: o = float(ort)
    except: o = float('inf')
        
    try: b = float(brkga)
    except: b = float('inf')
        
    if o == float('inf') and b == float('inf'):
        return "Nenhum (Falha)"
    if o < b:
        return "OR-Tools"
    elif b < o:
        return "BRKGA-CUDA"
    else:
        return "Empate"

def main():
    if not os.path.exists(INPUT_CSV):
        print(f"Erro: O arquivo {INPUT_CSV} não foi encontrado.")
        return

    with open(INPUT_CSV, 'r') as infile, open(OUTPUT_CSV, 'w', newline='') as outfile:
        reader = csv.DictReader(infile)
        
        # Novas colunas ajustadas para o TCC
        fieldnames = [
            "Instancia", "N", "BKS", 
            "ORT_Custo", "ORT_Tempo(s)", "Gap_ORT(%)", 
            "BRKGA_Custo", "BRKGA_Geracoes", "BRKGA_Tempo(s)", "Gap_BRKGA(%)",
            "Vencedor"
        ]
        
        writer = csv.DictWriter(outfile, fieldnames=fieldnames)
        writer.writeheader()
        
        for row in reader:
            inst = row["Instancia"]
            bks = get_bks(inst)
            
            ort_cost = row["ORT_Custo"]
            brkga_cost = row["BRKGA_Custo"]
            
            gap_ort = calc_gap(ort_cost, bks) if bks else "N/A"
            gap_brkga = calc_gap(brkga_cost, bks) if bks else "N/A"
            winner = determine_winner(ort_cost, brkga_cost)
            
            new_row = {
                "Instancia": inst,
                "N": row["N"],
                "BKS": int(bks) if bks else "N/A",
                "ORT_Custo": ort_cost,
                "ORT_Tempo(s)": row["ORT_Tempo(s)"],
                "Gap_ORT(%)": gap_ort,
                "BRKGA_Custo": brkga_cost,
                "BRKGA_Geracoes": row["BRKGA_Geracoes"],
                "BRKGA_Tempo(s)": row["BRKGA_Tempo(s)"],
                "Gap_BRKGA(%)": gap_brkga,
                "Vencedor": winner
            }
            writer.writerow(new_row)

    print(f"✅ Arquivo enriquecido gerado com sucesso em: {OUTPUT_CSV}")

if __name__ == '__main__':
    main()
