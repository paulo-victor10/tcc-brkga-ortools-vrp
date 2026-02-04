#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ortools/constraint_solver/routing.h"
#include "ortools/constraint_solver/routing_index_manager.h"
#include "ortools/constraint_solver/routing_parameters.h"

using operations_research::Assignment;
using operations_research::RoutingDimension;
using operations_research::RoutingIndexManager;
using operations_research::RoutingModel;
using operations_research::RoutingSearchParameters;

struct DataModel {
  const std::vector<std::vector<int64_t>> time_matrix = {
      {0, 6, 9, 8, 7, 3, 6, 2, 3, 2, 6, 6, 4, 4, 5, 9, 7},
      {6, 0, 8, 3, 2, 6, 8, 4, 8, 8, 13, 7, 5, 8, 12, 10, 14},
      {9, 8, 0, 11, 10, 6, 3, 9, 5, 8, 4, 15, 14, 13, 9, 18, 9},
      {8, 3, 11, 0, 1, 7, 10, 6, 10, 10, 14, 6, 7, 9, 14, 6, 16},
      {7, 2, 10, 1, 0, 6, 9, 4, 8, 9, 13, 4, 6, 8, 12, 8, 14},
      {3, 6, 6, 7, 6, 0, 2, 3, 2, 2, 7, 9, 7, 7, 6, 12, 8},
      {6, 8, 3, 10, 9, 2, 0, 6, 2, 5, 4, 12, 10, 10, 8, 15, 6},
      {2, 4, 9, 6, 4, 3, 6, 0, 4, 4, 8, 5, 4, 3, 7, 8, 10},
      {3, 8, 5, 10, 8, 2, 2, 4, 0, 3, 4, 9, 8, 7, 5, 13, 6},
      {2, 8, 8, 10, 9, 2, 5, 4, 3, 0, 4, 6, 5, 4, 3, 9, 5},
      {6, 13, 4, 14, 13, 7, 4, 8, 4, 4, 0, 10, 9, 8, 4, 13, 4},
      {6, 7, 15, 6, 4, 9, 12, 5, 9, 6, 10, 0, 1, 3, 7, 2, 10},
      {4, 5, 14, 7, 6, 7, 10, 4, 8, 5, 9, 1, 0, 2, 6, 3, 9},
      {4, 8, 13, 9, 8, 7, 10, 3, 7, 4, 8, 3, 2, 0, 4, 6, 7},
      {5, 12, 9, 14, 12, 6, 8, 7, 5, 3, 4, 7, 6, 4, 0, 9, 3},
      {9, 10, 18, 6, 8, 12, 15, 8, 13, 9, 13, 2, 3, 6, 9, 0, 12},
      {7, 14, 9, 16, 14, 8, 6, 10, 6, 5, 4, 10, 9, 7, 3, 12, 0},
  };

  const std::vector<std::pair<int64_t, int64_t>> time_windows = {
      {0, 5},   {7, 12},  {10, 15}, {5, 14},  {5, 13},  {0, 5},
      {5, 10},  {0, 10},  {5, 10},  {0, 5},   {10, 16}, {10, 15},
      {0, 5},   {5, 10},  {7, 8},   {10, 15}, {11, 15},
  };

  const int num_vehicles = 4;
  const RoutingIndexManager::NodeIndex depot{0};
};

static void PrintSolution(const DataModel& data,
                          const RoutingIndexManager& manager,
                          const RoutingModel& routing,
                          const Assignment& solution) {
  const RoutingDimension& time_dimension = routing.GetDimensionOrDie("Time");
  int64_t total_time = 0;

  for (int vehicle_id = 0; vehicle_id < data.num_vehicles; ++vehicle_id) {
    int64_t index = routing.Start(vehicle_id);
    std::ostringstream route;
    route << "Route for vehicle " << vehicle_id << ":\n";

    while (!routing.IsEnd(index)) {
      auto time_var = time_dimension.CumulVar(index);
      route << manager.IndexToNode(index).value()
            << " Time(" << solution.Min(time_var) << ", " << solution.Max(time_var)
            << ") -> ";
      index = solution.Value(routing.NextVar(index));
    }

    auto time_var = time_dimension.CumulVar(index);
    route << manager.IndexToNode(index).value()
          << " Time(" << solution.Min(time_var) << ", " << solution.Max(time_var)
          << ")\n";
    route << "Time of the route: " << solution.Min(time_var) << "min\n\n";

    std::cout << route.str();
    total_time += solution.Min(time_var);
  }

  std::cout << "Total time of all routes: " << total_time << "min\n";
  std::cout << "Solved in " << routing.solver()->wall_time() << "ms\n";
}

int main() {
  DataModel data;

  RoutingIndexManager manager(static_cast<int>(data.time_matrix.size()),
                              data.num_vehicles, data.depot);
  RoutingModel routing(manager);

  const int transit_callback_index = routing.RegisterTransitCallback(
      [&data, &manager](int64_t from_index, int64_t to_index) -> int64_t {
        const int from_node = manager.IndexToNode(from_index).value();
        const int to_node = manager.IndexToNode(to_index).value();
        return data.time_matrix[from_node][to_node];
      });

  routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);

  routing.AddDimension(transit_callback_index, int64_t{30}, int64_t{30}, false, "Time");

  const RoutingDimension& time_dimension = routing.GetDimensionOrDie("Time");

  for (int node = 1; node < static_cast<int>(data.time_windows.size()); ++node) {
    const int64_t index = manager.NodeToIndex(RoutingIndexManager::NodeIndex(node));
    time_dimension.CumulVar(index)->SetRange(data.time_windows[node].first,
                                             data.time_windows[node].second);
  }

  for (int vehicle = 0; vehicle < data.num_vehicles; ++vehicle) {
    const int64_t start_index = routing.Start(vehicle);
    time_dimension.CumulVar(start_index)->SetRange(data.time_windows[0].first,
                                                   data.time_windows[0].second);
    routing.AddVariableMinimizedByFinalizer(time_dimension.CumulVar(routing.Start(vehicle)));
    routing.AddVariableMinimizedByFinalizer(time_dimension.CumulVar(routing.End(vehicle)));
  }

  RoutingSearchParameters search_parameters = operations_research::DefaultRoutingSearchParameters();
  search_parameters.set_first_solution_strategy(
      operations_research::FirstSolutionStrategy::PATH_CHEAPEST_ARC);

  const Assignment* solution = routing.SolveWithParameters(search_parameters);
  if (!solution) {
    std::cout << "No solution found.\n";
    return 1;
  }

  PrintSolution(data, manager, routing, *solution);
  return 0;
}