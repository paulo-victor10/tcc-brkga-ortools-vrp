#include "CvrpDecoder.h"
#include <cmath>

CvrpDecoder::CvrpDecoder(const std::vector<std::pair<float, float>>& coords,
                         const std::vector<float>& customer_demands,
                         float vehicle_capacity)
    : demands(customer_demands), capacity(vehicle_capacity), num_nodes(coords.size()) {

    distances.resize(num_nodes * num_nodes, 0.0f);

    for (unsigned i = 0; i < num_nodes; ++i) {
        for (unsigned j = i + 1; j < num_nodes; ++j) {
            float dist = std::hypotf(coords[i].first - coords[j].first,
                                     coords[i].second - coords[j].second);
            distances[i * num_nodes + j] = dist;
            distances[j * num_nodes + i] = dist;
        }
    }
}

box::Fitness CvrpDecoder::decode(const box::Chromosome<box::GeneIndex>& tour) const {
    float fitness = 0.0f;
    float current_load = 0.0f;
    unsigned current_node = 0;
    const unsigned num_clients = config->chromosomeLength();

    for (unsigned i = 0; i < num_clients; ++i) {
        unsigned client = tour[i] + 1;
        float demand = demands[client];

        if (current_load + demand > capacity) {
            fitness += distances[current_node * num_nodes + 0];
            current_node = 0;
            current_load = 0.0f;
        }

        fitness += distances[current_node * num_nodes + client];
        current_load += demand;
        current_node = client;
    }

    fitness += distances[current_node * num_nodes + 0];

    return fitness;
}