# Relatório de Progresso – OR-Tools CVRP (Atualizado)

> **Objetivo do documento**  
Consolidar todo o progresso realizado desde o último relatório, registrar as principais decisões técnicas adotadas, descrever o estado atual estável do projeto e documentar o fluxo completo de experimentos, organização de pastas e métricas utilizadas, de forma a facilitar a reprodução dos resultados e a revisão futura no contexto do TCC.

---

## 1. Estado atual do projeto (checkpoint estável)

Este relatório descreve o estado **estável e funcional** do projeto após:
- rollback para um commit confiável;
- limpeza de tentativas experimentais instáveis;
- consolidação do pipeline CVRP com OR-Tools;
- execução completa de experimentos com métricas e CSVs.

**Observação:**  
Neste ponto não se busca capturar callbacks internos do solver (como o registro da melhor solução ao longo do tempo). O foco está na comparação por tempo limite fixo, na qualidade final da solução obtida e no gap em relação ao BKS, garantindo estabilidade experimental, reprodutibilidade e uma base consistente para comparação com o brkgaCUDA 2.0.

---

## 2. Estrutura atual do repositório (relevante)

```
tcc-brkga-ortools-vrp/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── parsers/
│   │   ├── vrplib_cvrp.h
│   │   └── vrplib_cvrp.cpp
│   └── ortools/
│       ├── cvrp_vrplib.cpp        # solver CVRP principal
│       ├── cvrp_demo.cpp          # toy
│       └── vrptw_demo.cpp         # toy
├── data/
│   ├── instances/
│   │   └── cvrp/
│   │       ├── A/
│   │       ├── B/
│   │       ├── P/
│   │       └── E/
│   └── manifests/
│       └── cvrp_experimental.txt
├── scripts/
│   ├── run_cvrp_manifest.sh
│   ├── parse_cvrp_logs_to_csv.py
│   └── enrich_cvrp_csv_with_bks.py
└── results/
    └── cvrp/
        ├── runs/
        │   ├── t2/
        │   └── t30/
        ├── summary_t2.csv
        ├── summary_t2_with_bks.csv
        ├── summary_t30.csv
        └── summary_t30_with_bks.csv
```

---

## 3. Instâncias utilizadas (manifesto CVRP)

Arquivo:
```
data/manifests/cvrp_experimental.txt
```

Instâncias **válidas, existentes e compatíveis com o parser**:

### Grupos utilizados
- **A (Augerat)** – baseline clássico
- **B (Augerat)** – médio porte
- **P (Augerat)** – relações extremas veículos/clientes
- **E (Christofides & Eilon)** – demandas grandes

### Grupos conscientemente excluídos
- **X** – instâncias muito grandes → OR-Tools não encontra solução inicial de forma confiável neste pipeline.

Essa exclusão foi uma decisão consciente, devidamente registrada, e considerada adequada para os objetivos da Fase 1.

---

## 4. Pipeline experimental atual (PASSO A PASSO)

### 4.1 Build (sempre que clonar ou alterar código)

```bash
cmake -S . -B build-linux \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/opt/ortools/or-tools_x86_64_Ubuntu-24.04_cpp_v9.10.4067"

cmake --build build-linux -j
```

---

### 4.2 Execução em lote (manifesto)

```bash
chmod +x scripts/run_cvrp_manifest.sh

./scripts/run_cvrp_manifest.sh data/manifests/cvrp_experimental.txt 2
./scripts/run_cvrp_manifest.sh data/manifests/cvrp_experimental.txt 30
```

O script realiza a leitura do manifesto, ignora comentários e linhas vazias, valida a existência dos arquivos `.vrp`, executa o solver para cada instância, salva os logs individualmente e registra eventuais arquivos ausentes ou falhas de execução.

---

### 4.3 Organização dos logs

```bash
mkdir -p results/cvrp/runs/t2
mkdir -p results/cvrp/runs/t30

mv results/cvrp/*_t2.log results/cvrp/runs/t2/
mv results/cvrp/*_t30.log results/cvrp/runs/t30/
```

---

### 4.4 Geração de CSV (parsing de logs)

```bash
python3 scripts/parse_cvrp_logs_to_csv.py \
  results/cvrp/runs/t2 \
  results/cvrp/summary_t2.csv

python3 scripts/parse_cvrp_logs_to_csv.py \
  results/cvrp/runs/t30 \
  results/cvrp/summary_t30.csv
```

CSV gerado contém:
- instance
- n_nodes
- capacity
- vehicles
- depot
- time_limit_s
- total_distance
- solved_ms
- log_file

---

### 4.5 Enriquecimento com BKS

```bash
python3 scripts/enrich_cvrp_csv_with_bks.py \
  results/cvrp/summary_t2.csv \
  results/cvrp/summary_t2_with_bks.csv \
  data/instances/cvrp

python3 scripts/enrich_cvrp_csv_with_bks.py \
  results/cvrp/summary_t30.csv \
  results/cvrp/summary_t30_with_bks.csv \
  data/instances/cvrp
```

Campos adicionados:
- **bks_cost**
- **gap_pct**

---

## 5. Métricas utilizadas (estado atual)

Para cada instância:

| Métrica | Descrição |
|------|---------|
| `total_distance` | custo final da solução |
| `time_limit_s` | tempo máximo permitido |
| `solved_ms` | tempo até término |
| `bks_cost` | melhor custo conhecido |
| `gap_pct` | gap percentual em relação ao BKS |

⚠️ **Não** é coletado no momento:
- melhor custo intermediário;
- tempo de melhora;
- curva tempo × qualidade.

Essas informações poderão ser coletadas em uma fase futura, mediante instrumentação adequada do solver, preservando a estabilidade do pipeline experimental.

---

## 6. Resultados obtidos (CVRP – OR-Tools)

### Comparação t = 2s vs t = 30s (GUIDED LOCAL SEARCH)

- Total de instâncias: **12**
- Melhorou em t30: **10**
- Igual: **2**
- Pior: **0**

Esses resultados indicam que o pipeline experimental está funcionando corretamente, que o aumento do tempo limite produz efeito mensurável na qualidade das soluções e que o uso da metaheurística está adequado ao objetivo proposto.

---

## 7. Decisões técnicas importantes (registradas)

### 7.1 Uso de `log_search`
A opção `log_search(true)` foi desativada. O principal motivo é o volume excessivo de saída gerada, que polui os logs, dificulta a análise posterior e inviabiliza o parsing automático. Como o interesse nesta fase está restrito aos resultados finais das execuções, essa opção não agrega valor prático.

---

### 7.2 Fallback de FirstSolutionStrategy

O fallback automático entre estratégias de solução inicial foi desativado. Essa decisão evita mascarar limitações do modelo ou do método e garante maior controle experimental, assegurando que cada execução respeite estritamente o tempo limite definido e utilize uma estratégia inicial fixa, permitindo uma comparação mais justa com o brkgaCUDA.

---

## 8. Estado final desta fase

### ✅ Concluído
- pipeline CVRP completo;
- execução em lote;
- logs organizados;
- CSVs limpos;
- enriquecimento com BKS;
- resultados consistentes;
- base **100% reprodutível**.

### ⏭ O que fazer em seguida:

1. Consolidar este marco no TCC
2. Replicar pipeline para **VRPTW (Homberger)**
3. Em fase posterior:
   - instrumentar coleta de melhor solução ao longo do tempo
   - comparar com brkgaCUDA 2.0

---
