#!/bin/bash

MANIFEST="data/manifests/qap_stress_granular.txt"

echo "========================================================"
echo "🔥 TESTE DE STRESS GRANULAR (OR-TOOLS / RAM vs TEMPO)"
echo "========================================================"

while IFS= read -r instance || [ -n "$instance" ]; do
    [[ -z "$instance" ]] && continue
    
    echo -e "\n🛑 Analisando o gargalo na instância: $instance"
    
    # Rodamos o OR-Tools. Não redirecionamos a saída para ver os erros ao vivo.
    ./build-linux/ortools_qap_poc data/instances/qap/$instance
    
    status=$?
    
    # Avaliador de danos:
    if [ $status -eq 137 ] || [ $status -eq 9 ]; then
        echo -e "\n💥 KILLED! COLAPSO DE MEMÓRIA (RAM) DETECTADO NA INSTÂNCIA $instance!"
        echo "A árvore de busca esgotou toda a memória do sistema."
        echo "EXPERIMENTO INTERROMPIDO. O teto de hardware foi descoberto."
        exit 1
    elif [ $status -eq 124 ] || [ $status -eq 143 ]; then
        echo -e "\n⏳ TIMEOUT! O OR-Tools estourou os 3 minutos, mas a RAM sobreviveu."
    elif [ $status -ne 0 ]; then
        echo -e "\n⚠️ Erro desconhecido (Código $status). Verifique a instância."
        exit 1
    else
        echo -e "\n✅ O OR-Tools conseguiu resolver dentro dos limites!"
    fi
    
    sleep 3
done < "$MANIFEST"

echo -e "\n========================================================"
echo "🏁 Teste concluído sem colapso fatal de RAM."
