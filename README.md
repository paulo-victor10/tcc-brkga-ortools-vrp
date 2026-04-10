## Objetivo do Projeto

Consolidar e avaliar a resolução dos problemas de **Roteamento de Veículos (CVRP)** e **Roteamento de Veículos com Janelas de Tempo (VRPTW)** utilizando o solver do Google OR-Tools.

Este repositório foca em:
- Estabilidade experimental
- Reprodutibilidade com tempo limite fixo
- Comparação de qualidade de solução (Gap) com as **Melhores Soluções Conhecidas (BKS)**

Também serve como base comparativa para o **brkgaCUDA 2.0**.

---

## 1. Estrutura do Repositório

```
tcc-brkga-ortools-vrp/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── parsers/
│   │   ├── vrplib_cvrp.h / .cpp
│   │   └── vrplib_vrptw.h / .cpp
│   └── ortools/
│       ├── cvrp_vrplib.cpp
│       ├── vrptw_vrplib.cpp
│       ├── cvrp_demo.cpp
│       └── vrptw_demo.cpp
├── data/
│   ├── instances/
│   │   └── cvrp/
│   └── manifests/
│       ├── cvrp_experimental.txt
│       └── vrptw_experimental.txt
├── scripts/
│   ├── run_cvrp_manifest.sh
│   ├── parse_cvrp_logs_to_csv.py
│   └── enrich_cvrp_csv_with_bks.py
└── results/
    └── cvrp/
        ├── runs/
        └── *.csv
```

---

## 2. Configuração do Ambiente (Ubuntu 24.04 / WSL2)

O projeto utiliza:
- **OR-Tools v9.10**
- **C++17**

Como não há binários oficiais para Ubuntu 24.04 nessa versão, utilizamos o de **Ubuntu 22.04**, que é compatível.

### 2.1 Dependências

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake wget tar python3 python3-pip
```

### 2.2 Instalação do OR-Tools

```bash
sudo mkdir -p /opt/ortools

wget https://github.com/google/or-tools/releases/download/v9.10/or-tools_amd64_ubuntu-22.04_cpp_v9.10.4067.tar.gz

sudo tar -xzvf or-tools_amd64_ubuntu-22.04_cpp_v9.10.4067.tar.gz -C /opt/ortools/

sudo mv /opt/ortools/or-tools_x86_64_Ubuntu-22.04_cpp_v9.10.4067 \
        /opt/ortools/or-tools_x86_64_Ubuntu-24.04_cpp_v9.10.4067

rm or-tools_amd64_ubuntu-22.04_cpp_v9.10.4067.tar.gz
```

---

## 3. Compilação (Build)

### Permissões dos scripts

```bash
chmod +x scripts/*.sh
```

### Build do projeto

```bash
cmake -S . -B build-linux \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="/opt/ortools/or-tools_x86_64_Ubuntu-24.04_cpp_v9.10.4067"

cmake --build build-linux -j$(nproc)
```

---

## 4. Pipeline Experimental

As instâncias estão em:

```
data/manifests/cvrp_experimental.txt
```

### Observação

Instâncias muito grandes do grupo **X** foram excluídas, pois o OR-Tools não encontra solução inicial de forma confiável.

---

### 4.1 Execução em lote (CVRP)

```bash
mkdir -p results/cvrp/runs/t2
mkdir -p results/cvrp/runs/t30

./scripts/run_cvrp_manifest.sh data/manifests/cvrp_experimental.txt 2
./scripts/run_cvrp_manifest.sh data/manifests/cvrp_experimental.txt 30
```

---

### 4.2 Organização dos logs

```bash
mv results/cvrp/*_t2.log results/cvrp/runs/t2/
mv results/cvrp/*_t30.log results/cvrp/runs/t30/
```

---

### 4.3 Geração de CSV e cálculo de Gap

#### Parsing dos logs

```bash
python3 scripts/parse_cvrp_logs_to_csv.py results/cvrp/runs/t2 results/cvrp/summary_t2.csv
python3 scripts/parse_cvrp_logs_to_csv.py results/cvrp/runs/t30 results/cvrp/summary_t30.csv
```

#### Enriquecimento com BKS (Gap %)

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

---

## 5. Decisões Técnicas

### Desativação de fallback inicial
- Remove troca automática de estratégias
- Garante controle experimental rigoroso

### Log Search desativado
- Evita poluição de logs
- Facilita parsing automatizado

### Tipagem forte (C++ / OR-Tools)
- Uso de `RoutingIndexManager::NodeIndex()`
- Evita erros entre índices e dimensões

### Ajuste no CMake
```cmake
target_include_directories(vrptw_vrplib PRIVATE src)
```

---
