#include "CvrpDecoder.h"
#include "parsers/vrplib_cvrp.h"
#include <brkga-cuda/Brkga.hpp>
#include <brkga-cuda/BrkgaConfiguration.hpp>
#include <brkga-cuda/DecodeType.hpp>
#include <iostream>
#include <vector>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_instancia.vrp>\n";
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

    unsigned maxGenerations = 1000;
    unsigned exchangeInterval = 25;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (unsigned g = 1; g <= maxGenerations; ++g) {
        brkga.evolve();
        if (g % exchangeInterval == 0) {
            brkga.exchangeElites();
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Distance: " << brkga.getBestFitness() << "\n";
    std::cout << "Time (ms): " << duration_ms << "\n";

    return 0;
}