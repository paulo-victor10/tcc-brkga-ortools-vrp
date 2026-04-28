#include "QapDecoder.h"
#include <algorithm>

QapDecoder::QapDecoder(int size, const std::vector<std::vector<int>>& dist, const std::vector<std::vector<int>>& flow)
    : n(size), distance_matrix(dist), flow_matrix(flow) {}

box::Fitness QapDecoder::decode(const box::Chromosome<box::GeneIndex>& chr) const {
    
    // OTIMIZAÇÃO EXTREMA: Array na Stack. 
    // Zero alocação de memória dinâmica. Impossível vazar memória.
    int permutation[256];
    
    for(int i = 0; i < n; ++i) {
        permutation[i] = i;
    }

    // Ordenamos os índices baseados nas chaves da GPU
    std::sort(permutation, permutation + n, [&chr](int a, int b) {
        return chr[a] < chr[b];
    });

    long long total_cost = 0;
    
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (flow_matrix[i][j] > 0) {
                total_cost += flow_matrix[i][j] * distance_matrix[permutation[i]][permutation[j]];
            }
        }
    }

    return static_cast<float>(total_cost);
}