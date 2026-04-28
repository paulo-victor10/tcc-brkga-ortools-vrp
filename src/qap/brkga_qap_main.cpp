#include "QapDecoder.h"
#include <brkga-cuda/Brkga.hpp>
#include <brkga-cuda/BrkgaConfiguration.hpp>
#include <brkga-cuda/DecodeType.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

// Função auxiliar para carregar a instância QAPLIB
bool LoadQAP(const std::string& path, int& n, 
             std::vector<std::vector<int>>& dist, 
             std::vector<std::vector<int>>& flow) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    file >> n;
    dist.assign(n, std::vector<int>(n));
    flow.assign(n, std::vector<int>(n));
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) file >> dist[i][j];
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) file >> flow[i][j];
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Uso: ./brkga_qap <instancia> <tempo_segundos>" << std::endl;
        return 1;
    }

    std::string instance_path = argv[1];
    double max_time = std::stod(argv[2]);

    int n;
    std::vector<std::vector<int>> dist, flow;
    if (!LoadQAP(instance_path, n, dist, flow)) {
        std::cerr << "Erro ao carregar instancia!" << std::endl;
        return 1;
    }

    // Inicializa o decodificador passando os dados do QAP
    QapDecoder decoder(n, dist, flow);

    // Tipo de decodificação no C++ (mesmo do CVRP)
    auto dt = box::DecodeType::fromString("cpu-permutation");

    // Usa a mesma estrutura de configuração (Builder) que funcionou no CVRP
    auto config = box::BrkgaConfiguration::Builder()
                      .decoder(&decoder)
                      .decodeType(dt)
                      .numberOfPopulations(1)
                      .populationSize(1024)
                      .chromosomeLength(n)
                      .numberOfElites(150)
                      .numberOfMutants(100)
                      .parents(2, box::Bias::CONSTANT, 1)
                      .numberOfElitesToExchange(0)
                      .seed(1)
                      .gpuThreads(256)
                      .ompThreads(6)
                      .build();

    box::Brkga brkga(config);

    std::cout << "Iniciando BRKGA-CUDA para QAP (N=" << n << ")..." << std::endl;
    
    auto start_time = std::chrono::steady_clock::now();
    unsigned generation = 0;
    float best_fitness = std::numeric_limits<float>::max();

    while (true) {
        brkga.evolve();
        generation++;

        float current_best = brkga.getBestFitness();
        if (current_best < best_fitness) {
            best_fitness = current_best;
            std::cout << "Ger " << generation << " | Melhor Custo: " << best_fitness << std::endl;
        }

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();
        if (elapsed >= max_time) break;
    }

    std::cout << "\n--- RESULTADO BRKGA-CUDA ---" << std::endl;
    std::cout << "Custo Final: " << best_fitness << std::endl;
    std::cout << "Geracoes: " << generation << std::endl;
    std::cout << "Tempo: " << max_time << "s" << std::endl;

    return 0;
}