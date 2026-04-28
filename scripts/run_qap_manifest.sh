#!/bin/bash

# Define os caminhos a partir da raiz do projeto
MANIFEST="data/manifests/qap_experimental.txt"
RESULTS_ORTOOLS="results/qap/runs/ortools"
RESULTS_BRKGA="results/qap/runs/brkga"

# Compila para garantir que tudo está atualizado
echo "⚙️ Compilando executáveis do QAP..."
cd build-linux
make brkga_qap ortools_qap_poc -j$(nproc)
cd ..

echo "================================================"
echo "🚀 INICIANDO BATERIA DO MANIFESTO QAP"
echo "================================================"

# Loop de leitura do manifesto
while IFS= read -r instance || [ -n "$instance" ]; do
    # Ignora linhas vazias
    [[ -z "$instance" ]] && continue
    
    echo -e "\n📌 Processando Instância: $instance"
    
    ORTOOLS_LOG="$RESULTS_ORTOOLS/${instance}.log"
    BRKGA_LOG="$RESULTS_BRKGA/${instance}.log"
    
    echo "   🧠 [1/2] Rodando OR-Tools (Limite 180s)..."
    ./build-linux/ortools_qap_poc data/instances/qap/$instance > $ORTOOLS_LOG
    
    # Pausa para o sistema operacional limpar a memória RAM
    sleep 3 
    
    echo "   ⚡ [2/2] Rodando BRKGA-CUDA (Limite 180s)..."
    ./build-linux/brkga_qap data/instances/qap/$instance 180 > $BRKGA_LOG
    
    sleep 3
done < "$MANIFEST"

echo -e "\n================================================"
echo "🎉 TODOS OS EXPERIMENTOS CONCLUÍDOS COM SUCESSO!"
echo "📂 Verifique os logs na pasta: results/qap/runs/"
