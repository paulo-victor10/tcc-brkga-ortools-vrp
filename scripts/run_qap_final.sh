#!/bin/bash

MANIFEST="data/manifests/qap_final_experiment.txt"
RESULTS_DIR="results/qap/runs"

mkdir -p $RESULTS_DIR/ortools
mkdir -p $RESULTS_DIR/brkga

echo "⚙️ Compilando executáveis para garantir a versão de 5 minutos..."
cd build-linux
make brkga_qap ortools_qap_poc -j$(nproc)
cd ..

echo "========================================================"
echo "🚀 INICIANDO BATERIA DO EXPERIMENTO QAP (RESUMÍVEL)"
echo "========================================================"

while IFS= read -r instance || [ -n "$instance" ]; do
    [[ -z "$instance" ]] && continue
    
    ORTOOLS_LOG="$RESULTS_DIR/ortools/${instance}.log"
    BRKGA_LOG="$RESULTS_DIR/brkga/${instance}.log"
    
    # ---------------------------------------------------------
    # MECANISMO DE RESILIÊNCIA (Retoma de onde parou)
    # Se os dois logs já existem e têm tamanho maior que zero (-s), ele pula.
    # ---------------------------------------------------------
    if [ -s "$ORTOOLS_LOG" ] && [ -s "$BRKGA_LOG" ]; then
        echo "⏭️ Pulando: $instance (Logs já existem e estão completos)"
        continue
    fi
    
    echo -e "\n📌 Processando: $instance"
    
    echo "   🧠 [1/2] OR-Tools (Limite 300s)..."
    ./build-linux/ortools_qap_poc data/instances/qap/$instance > "$ORTOOLS_LOG"
    
    echo "   💤 Resfriando o hardware e limpando a RAM (10s)..."
    sleep 10
    
    echo "   ⚡ [2/2] BRKGA-CUDA (Limite 300s)..."
    # Passando 300 segundos fixos como parâmetro para o BRKGA
    ./build-linux/brkga_qap data/instances/qap/$instance 300 > "$BRKGA_LOG"
    
    echo "   💤 Resfriando a placa de vídeo (10s)..."
    sleep 10
    
done < "$MANIFEST"

echo -e "\n========================================================"
echo "🎉 BATERIA FINALIZADA COM SUCESSO! Logs salvos em $RESULTS_DIR"
echo "========================================================"
