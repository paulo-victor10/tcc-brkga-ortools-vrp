#include "CvrpDecoder.h"
#include "parsers/vrplib_cvrp.h"
#include <brkga-cuda/Brkga.hpp>
#include <brkga-cuda/BrkgaConfiguration.hpp>
#include <brkga-cuda/DecodeType.hpp>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <string>
#include <regex>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <caminho_instancia.vrp> <tempo_limite_segundos>\n";
        return 1;
    }

    auto problem = LoadVRPLIB_CVRP(argv[1]);

    std::vector<std::pair<float, float>> coords(problem.dimension);
    std::vector<float> demands(problem.dimension);

    for (int i = 0; i < problem.dimension; ++i) {
        coords[i] = { static_cast<float>(problem.x[i]), static_cast<float>(problem.y[i]) };
        demands[i] = static_cast<float>(problem.demand[i]);
    }

    float capacity = static_cast<float>(problem.capacity);
    unsigned num_clients = problem.dimension - 1;

    CvrpDecoder decoder(coords, demands, capacity);

    auto dt = box::DecodeType::fromString("cpu-permutation");

    auto config = box::BrkgaConfiguration::Builder()
                      .decoder(&decoder)
                      .decodeType(dt)
                      .numberOfPopulations(3)
                      .populationSize(256)
                      .chromosomeLength(num_clients)
                      .numberOfElites(25)
                      .numberOfMutants(25)
                      .parents(2, box::Bias::CONSTANT, 1)
                      .numberOfElitesToExchange(3)
                      .seed(1)
                      .gpuThreads(256)
                      .ompThreads(6)
                      .build();

    box::Brkga brkga(config);

    double time_limit_s = std::stod(argv[2]);
    unsigned exchangeInterval = 25;
    unsigned g = 1;

    auto start_time = std::chrono::high_resolution_clock::now();

    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current_time - start_time;
        
        if (elapsed.count() >= time_limit_s) {
            break;
        }

        brkga.evolve();
        if (g % exchangeInterval == 0) {
            brkga.exchangeElites();
        }
        g++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::string filename = argv[1];
    int vehicles = 0;
    std::smatch match;
    if (std::regex_search(filename, match, std::regex("-k(\\d+)"))) {
        vehicles = std::stoi(match[1].str());
    }

    std::cout << "Instance: " << problem.dimension << " nodes, capacity " << problem.capacity 
              << ", vehicles " << vehicles << ", depot " << problem.depot << "\n";
    std::cout << "Total distance: " << static_cast<int>(std::round(brkga.getBestFitness())) << "\n";
    std::cout << "Solved in " << duration_ms << " ms\n";
    std::cout << "Generations completed: " << g << "\n";

    return 0;
}