#!/bin/bash

MANIFEST="data/manifests/qap_stress_test.txt"

echo "================================================"
echo "🔥 INICIANDO TESTE DE STRESS DE MEMÓRIA (RAM)"
echo "================================================"

while IFS= read -r instance || [ -n "$instance" ]; do
    [[ -z "$instance" ]] && continue
    
    echo -e "\n🛑 Testando Limite com: $instance"
    
    # Rodamos o OR-Tools sem redirecionar a saída de erro, para vermos o "Killed" na hora
    ./build-linux/ortools_qap_poc data/instances/qap/$instance
    
    # Se o comando falhar (Killed), o bash retorna código de erro diferente de 0 e 124 (timeout)
    status=$?
    if [ $status -eq 137 ] || [ $status -eq 9 ]; then
        echo "💥 COLAPSO DE MEMÓRIA DETECTADO (OOM Killer) NA INSTÂNCIA $instance!"
        echo "EXPERIMENTO INTERROMPIDO. O limite do hardware foi encontrado."
        exit 1
    fi
    
    sleep 2
done < "$MANIFEST"

echo "Sobreviveu a todas as instâncias! (Improvável)"
