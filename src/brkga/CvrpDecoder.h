#ifndef CVRP_DECODER_H
#define CVRP_DECODER_H

#include <brkga-cuda/Decoder.hpp>
#include <vector>
#include <utility>

class CvrpDecoder : public box::Decoder {
private:
    std::vector<float> distances;
    std::vector<float> demands;
    float capacity;
    unsigned num_nodes;

    // Matriz de Vizinhança para a Busca Granular
    std::vector<std::vector<unsigned>> nearest_neighbors;

    // Métodos Meméticos
    void optimizeGranularInterRoute(std::vector<std::vector<unsigned>>& routes) const;
    float optimizeRoute2Opt(std::vector<unsigned>& route) const;

public:
    using box::Decoder::decode;

    CvrpDecoder(const std::vector<std::pair<float, float>>& coords,
                const std::vector<float>& customer_demands,
                float vehicle_capacity);

    box::Fitness decode(const box::Chromosome<box::GeneIndex>& tour) const override;
};

#endif