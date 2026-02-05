#include <cctype>
#include <cstdint>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "ortools/constraint_solver/constraint_solver.h"
#include "ortools/constraint_solver/routing.h"
#include "ortools/constraint_solver/routing_index_manager.h"
#include "ortools/constraint_solver/routing_parameters.h"

#include "parsers/vrplib_cvrp.h"

using operations_research::Assignment;
using operations_research::RoutingDimension;
using operations_research::RoutingIndexManager;
using operations_research::RoutingModel;
using operations_research::RoutingSearchParameters;

static int ParseVehiclesFromFilename(const std::string& path) {
  std::string name = path;
  const auto slash = name.find_last_of("/\\");
  if (slash != std::string::npos) name = name.substr(slash + 1);
  const auto dot = name.rfind('.');
  if (dot != std::string::npos) name = name.substr(0, dot);

  const auto kpos = name.rfind("-k");
  if (kpos == std::string::npos || kpos + 2 >= name.size()) return -1;

  int v = 0;
  for (size_t i = kpos + 2; i < name.size(); ++i) {
    if (!std::isdigit(static_cast<unsigned char>(name[i]))) break;
    v = v * 10 + (name[i] - '0');
  }
  return (v > 0) ? v : -1;
}

static int64_t Euc2dRounded(double x1, double y1, double x2, double y2) {
  const double dx = x1 - x2;
  const double dy = y1 - y2;
  const double d = std::sqrt(dx * dx + dy * dy);
  return static_cast<int64_t>(std::floor(d + 0.5));
}

static std::vector<std::vector<int64_t>> BuildDistanceMatrix(const CVRPInstance& inst) {
  const int n = inst.dimension;
  std::vector<std::vector<int64_t>> mat(n, std::vector<int64_t>(n, 0));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      mat[i][j] = Euc2dRounded(inst.x[i], inst.y[i], inst.x[j], inst.y[j]);
    }
  }
  return mat;
}

static void PrintSolution(const CVRPInstance& inst,
                          int num_vehicles,
                          const RoutingIndexManager& manager,
                          const RoutingModel& routing,
                          const Assignment& solution,
                          int64_t total_cost) {
  const RoutingDimension& cap = routing.GetDimensionOrDie("Capacity");

  std::cout << "Instance: " << inst.dimension << " nodes, capacity " << inst.capacity
            << ", vehicles " << num_vehicles << ", depot " << inst.depot << "\n";
  std::cout << "Total distance: " << total_cost << "\n";
  std::cout << "Solved in " << routing.solver()->wall_time() << "ms\n\n";

  for (int v = 0; v < num_vehicles; ++v) {
    int64_t index = routing.Start(v);
    std::ostringstream route;
    int64_t route_cost = 0;

    route << "Route for vehicle " << v << ":\n";

    while (!routing.IsEnd(index)) {
      const int node = manager.IndexToNode(index).value();
      const auto load_var = cap.CumulVar(index);
      route << node << " Load(" << solution.Value(load_var) << ") -> ";
      const int64_t prev = index;
      index = solution.Value(routing.NextVar(index));
      route_cost += routing.GetArcCostForVehicle(prev, index, v);
    }

    const int end_node = manager.IndexToNode(index).value();
    const auto load_var = cap.CumulVar(index);
    route << end_node << " Load(" << solution.Value(load_var) << ")\n";
    route << "Distance of the route: " << route_cost << "\n\n";

    std::cout << route.str();
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: cvrp_vrplib <instance.vrp> [num_vehicles] [time_limit_seconds]\n";
    return 1;
  }

  const std::string instance_path = argv[1];
  CVRPInstance inst = LoadVRPLIB_CVRP(instance_path);

  int num_vehicles = -1;
  if (argc >= 3) {
    num_vehicles = std::stoi(argv[2]);
  } else {
    num_vehicles = ParseVehiclesFromFilename(instance_path);
  }
  if (num_vehicles <= 0) {
    std::cerr << "Could not determine num_vehicles. Provide it as 2nd argument.\n";
    return 1;
  }

  int time_limit_s = 2;
  if (argc >= 4) time_limit_s = std::stoi(argv[3]);

  const auto distance_matrix = BuildDistanceMatrix(inst);

  RoutingIndexManager manager(inst.dimension, num_vehicles,
                              RoutingIndexManager::NodeIndex(inst.depot));
  RoutingModel routing(manager);

  const int transit_callback_index = routing.RegisterTransitCallback(
      [&distance_matrix, &manager](int64_t from_index, int64_t to_index) -> int64_t {
        const int from_node = manager.IndexToNode(from_index).value();
        const int to_node = manager.IndexToNode(to_index).value();
        return distance_matrix[from_node][to_node];
      });

  routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);

  const int demand_callback_index = routing.RegisterUnaryTransitCallback(
      [&inst, &manager](int64_t from_index) -> int64_t {
        const int from_node = manager.IndexToNode(from_index).value();
        return inst.demand[from_node];
      });

  routing.AddDimensionWithVehicleCapacity(
      demand_callback_index,
      int64_t{0},
      std::vector<int64_t>(num_vehicles, inst.capacity),
      true,
      "Capacity");

  RoutingSearchParameters search_parameters =
      operations_research::DefaultRoutingSearchParameters();

  search_parameters.set_first_solution_strategy(
      operations_research::FirstSolutionStrategy::PARALLEL_CHEAPEST_INSERTION);

  search_parameters.set_local_search_metaheuristic(
      operations_research::LocalSearchMetaheuristic::GUIDED_LOCAL_SEARCH);

  search_parameters.set_use_unfiltered_first_solution_strategy(true);
  search_parameters.set_use_full_propagation(true);
  search_parameters.set_log_search(false);
  search_parameters.mutable_time_limit()->set_seconds(time_limit_s);

  int64_t best_cost = std::numeric_limits<int64_t>::max();
  int64_t best_time_ms = -1;

  routing.AddAtSolutionCallback([&]() {
    const int64_t current_cost = routing.CostVar()->Value();
    if (current_cost < best_cost) {
      best_cost = current_cost;
      best_time_ms = routing.solver()->wall_time();
    }
  });

  const Assignment* solution = routing.SolveWithParameters(search_parameters);

  if (!solution) {
    std::cout << "No solution found.\n";
    return 1;
  }

  int64_t total_cost = 0;
  for (int v = 0; v < num_vehicles; ++v) {
    int64_t index = routing.Start(v);
    while (!routing.IsEnd(index)) {
      const int64_t next = solution->Value(routing.NextVar(index));
      total_cost += routing.GetArcCostForVehicle(index, next, v);
      index = next;
    }
  }

  std::cout << "Best distance (during search): " << best_cost << "\n";
  std::cout << "Best found at: " << best_time_ms << "ms\n\n";

  PrintSolution(inst, num_vehicles, manager, routing, *solution, total_cost);
  return 0;
}
