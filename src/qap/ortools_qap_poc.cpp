#include "ortools/sat/cp_model.h"
#include "ortools/sat/model.h"
#include "ortools/sat/cp_model.pb.h"
#include "ortools/sat/cp_model_solver.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace operations_research;
using namespace operations_research::sat;

// Função para ler instâncias da QAPLIB (formato clássico)
bool ReadQAPLib(const std::string& filename, int& n, 
                std::vector<std::vector<int>>& distance, 
                std::vector<std::vector<int>>& flow) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    file >> n;
    distance.assign(n, std::vector<int>(n));
    flow.assign(n, std::vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> distance[i][j];

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> flow[i][j];

    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: ./ortools_qap_poc <caminho_para_instancia_qap>" << std::endl;
        return 1;
    }

    int n;
    std::vector<std::vector<int>> distance, flow;
    if (!ReadQAPLib(argv[1], n, distance, flow)) {
        std::cerr << "Erro ao ler a instancia." << std::endl;
        return 1;
    }

    std::cout << "Instancia QAP carregada. N = " << n << std::endl;

    // 1. Inicializa o Builder do CP-SAT
    CpModelBuilder cp_model;

    // 2. Variáveis de Decisão: x[i] representa a 'localização' atribuída à 'fábrica' i
    std::vector<IntVar> x;
    for (int i = 0; i < n; ++i) {
        x.push_back(cp_model.NewIntVar({0, n - 1}).WithName("x_" + std::to_string(i)));
    }

    // 3. Restrição Principal: Nenhuma fábrica pode ficar no mesmo lugar
    cp_model.AddAllDifferent(x);

    // 4. O "Truque" de Indexação CP-SAT: Achatando a Matriz de Distância
    std::vector<int64_t> flat_distance;
    int64_t max_dist = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            flat_distance.push_back(distance[i][j]);
            if (distance[i][j] > max_dist) max_dist = distance[i][j];
        }
    }

    // DECLARED ONCE: A expressão linear da nossa função objetivo
    LinearExpr objective;

    // 5. Construindo a Função Objetivo Quadrática
    // Custo = Somatório( Fluxo[i][j] * Distancia[x[i]][x[j]] )
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (flow[i][j] > 0) {
                // Acha o índice unidimensional dinâmico: idx = x[i] * n + x[j]
                IntVar idx = cp_model.NewIntVar({0, n * n - 1});
                cp_model.AddEquality(idx, x[i] * n + x[j]);

                // Extrai a distância usando a restrição "Element" (Lookup na tabela)
                IntVar dist_ij = cp_model.NewIntVar({0, max_dist});
                cp_model.AddElement(idx, flat_distance, dist_ij);

                // Adiciona ao custo total usando sobrecarga de operador
                objective += dist_ij * flow[i][j];
            }
        }
    }

    cp_model.Minimize(objective);

    // 6. Configuração e Execução do Solver
    Model model;
    SatParameters parameters;
    parameters.set_max_time_in_seconds(300.0); // Limite de 3 minutos
    parameters.set_log_search_progress(true);  // Mostra a árvore de busca no terminal
    model.Add(NewSatParameters(parameters));

    std::cout << "Iniciando a busca no OR-Tools (Limite: 3 mins)..." << std::endl;
    const CpSolverResponse response = SolveCpModel(cp_model.Build(), &model);

    std::cout << "\n--- RESULTADO OR-TOOLS ---" << std::endl;
    std::cout << "Status: " << response.status() << std::endl;
    if (response.status() == CpSolverStatus::OPTIMAL || response.status() == CpSolverStatus::FEASIBLE) {
        std::cout << "Custo (Distancia Total): " << response.objective_value() << std::endl;
    }
    std::cout << "Tempo decorrido: " << response.wall_time() << "s" << std::endl;

    return 0;
}