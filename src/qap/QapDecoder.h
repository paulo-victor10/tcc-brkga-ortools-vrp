#ifndef QAP_DECODER_H
#define QAP_DECODER_H

#include <brkga-cuda/Decoder.hpp>
#include <vector>

class QapDecoder : public box::Decoder {
private:
    int n;
    std::vector<std::vector<int>> distance_matrix;
    std::vector<std::vector<int>> flow_matrix;

public:
    using box::Decoder::decode;

    QapDecoder(int size, const std::vector<std::vector<int>>& dist, const std::vector<std::vector<int>>& flow);

    box::Fitness decode(const box::Chromosome<box::GeneIndex>& chr) const override;
};

#endif