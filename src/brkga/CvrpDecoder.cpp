#include "CvrpDecoder.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <random> // Biblioteca nativa de aleatoriedade

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

    unsigned K = std::min(15u, num_nodes - 1);
    nearest_neighbors.resize(num_nodes);
    
    for (unsigned i = 1; i < num_nodes; ++i) { 
        std::vector<std::pair<float, unsigned>> dist_node;
        for (unsigned j = 1; j < num_nodes; ++j) {
            if (i != j) dist_node.push_back({distances[i * num_nodes + j], j});
        }
        std::sort(dist_node.begin(), dist_node.end());
        for (unsigned k = 0; k < K; ++k) {
            nearest_neighbors[i].push_back(dist_node[k].second);
        }
    }
}

void CvrpDecoder::optimizeGranularInterRoute(std::vector<std::vector<unsigned>>& routes) const {
    std::vector<int> node_to_route(num_nodes, -1);
    std::vector<int> node_to_pos(num_nodes, -1);
    std::vector<float> route_loads(routes.size(), 0.0f);

    for (size_t r = 0; r < routes.size(); ++r) {
        for (size_t p = 1; p < routes[r].size() - 1; ++p) {
            unsigned node = routes[r][p];
            node_to_route[node] = r;
            node_to_pos[node] = p;
            route_loads[r] += demands[node];
        }
    }

    bool improvement = true;
    while (improvement) {
        improvement = false;
        
        for (size_t r1 = 0; r1 < routes.size(); ++r1) {
            for (size_t i = 1; i < routes[r1].size() - 1; ++i) {
                unsigned u = routes[r1][i];
                unsigned prev_u = routes[r1][i-1];
                unsigned next_u = routes[r1][i+1];
                
                float delta_rem = distances[prev_u * num_nodes + next_u] 
                                - distances[prev_u * num_nodes + u] 
                                - distances[u * num_nodes + next_u];

                for (unsigned v : nearest_neighbors[u]) {
                    int r2 = node_to_route[v];
                    if (r2 == -1 || r2 == r1) continue; 

                    if (route_loads[r2] + demands[u] > capacity) continue; 

                    int j = node_to_pos[v]; 
                    
                    unsigned next_v = routes[r2][j+1];
                    float delta_add = distances[v * num_nodes + u] 
                                    + distances[u * num_nodes + next_v] 
                                    - distances[v * num_nodes + next_v];

                    if (delta_rem + delta_add < -1e-4f) {
                        routes[r2].insert(routes[r2].begin() + j + 1, u);
                        routes[r1].erase(routes[r1].begin() + i);

                        route_loads[r1] -= demands[u];
                        route_loads[r2] += demands[u];

                        for (size_t p = 1; p < routes[r1].size() - 1; ++p) {
                            node_to_route[routes[r1][p]] = r1; node_to_pos[routes[r1][p]] = p;
                        }
                        for (size_t p = 1; p < routes[r2].size() - 1; ++p) {
                            node_to_route[routes[r2][p]] = r2; node_to_pos[routes[r2][p]] = p;
                        }

                        improvement = true;
                        break; 
                    }
                }
                if (improvement) break; 
            }
            if (improvement) break; 
        }
    }

    routes.erase(std::remove_if(routes.begin(), routes.end(),
        [](const std::vector<unsigned>& r) { return r.size() <= 2; }), routes.end());
}

float CvrpDecoder::optimizeRoute2Opt(std::vector<unsigned>& route) const {
    if (route.size() <= 3) {
        float cost = 0.0f;
        for (size_t i = 0; i < route.size() - 1; ++i) {
            cost += distances[route[i] * num_nodes + route[i+1]];
        }
        return cost;
    }

    bool improvement = true;
    while (improvement) {
        improvement = false;
        for (size_t i = 0; i < route.size() - 2; ++i) {
            for (size_t j = i + 2; j < route.size() - 1; ++j) {
                unsigned A = route[i];
                unsigned B = route[i+1];
                unsigned C = route[j];
                unsigned D = route[j+1];

                float current_dist = distances[A * num_nodes + B] + distances[C * num_nodes + D];
                float new_dist = distances[A * num_nodes + C] + distances[B * num_nodes + D];

                if (new_dist < current_dist - 1e-4f) {
                    std::reverse(route.begin() + i + 1, route.begin() + j + 1);
                    improvement = true;
                }
            }
        }
    }

    float final_cost = 0.0f;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        final_cost += distances[route[i] * num_nodes + route[i+1]];
    }
    return final_cost;
}

box::Fitness CvrpDecoder::decode(const box::Chromosome<box::GeneIndex>& tour) const {
    const unsigned num_clients = config->chromosomeLength();
    
    // FASE 1: Split de Prins
    std::vector<float> V(num_clients + 1, std::numeric_limits<float>::max());
    std::vector<int> P(num_clients + 1, -1);
    V[0] = 0.0f;

    for (unsigned i = 0; i < num_clients; ++i) {
        float current_load = 0.0f;
        float current_cost = 0.0f;
        unsigned last_client = 0; 

        for (unsigned j = i + 1; j <= num_clients; ++j) {
            unsigned client = tour[j - 1] + 1;
            current_load += demands[client];

            if (current_load > capacity) break; 

            if (j == i + 1) {
                current_cost = distances[0 * num_nodes + client] + distances[client * num_nodes + 0];
            } else {
                current_cost = current_cost - distances[last_client * num_nodes + 0] 
                             + distances[last_client * num_nodes + client] 
                             + distances[client * num_nodes + 0];
            }

            if (V[i] + current_cost < V[j]) {
                V[j] = V[i] + current_cost;
                P[j] = i;
            }
            last_client = client;
        }
    }

    std::vector<std::vector<unsigned>> routes;
    int curr = num_clients;
    while (curr > 0) {
        int prev = P[curr];
        std::vector<unsigned> route;
        route.push_back(0); 
        for (int k = prev; k < curr; ++k) {
            route.push_back(tour[k] + 1);
        }
        route.push_back(0);
        routes.push_back(route);
        curr = prev;
    }

    // FASE 2: O PONTO DE EQUILÍBRIO (0,5% de Busca Granular)
    
    // Aleatoriedade isolada por thread (Thread-safe) para não causar gargalo no OpenMP
    thread_local std::mt19937 rng(std::random_device{}());
    
    // Roleta de 0 a 999. Para termos 0,5%, o número sorteado deve ser menor que 5.
    std::uniform_int_distribution<int> dist(0, 999);

    if (dist(rng) < 5) {
        optimizeGranularInterRoute(routes); // A Busca Pesada Cruza-Caminhões
    }
    
    // O polimento de rua (2-opt) roda para 100% dos indivíduos, pois é quase instantâneo
    float total_fitness = 0.0f;
    for (auto& route : routes) {
        total_fitness += optimizeRoute2Opt(route); 
    }

    return total_fitness;
}