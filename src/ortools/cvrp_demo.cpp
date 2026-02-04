#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ortools/constraint_solver/routing.h"
#include "ortools/constraint_solver/routing_index_manager.h"
#include "ortools/constraint_solver/routing_parameters.h"

using operations_research::Assignment;
using operations_research::RoutingIndexManager;
using operations_research::RoutingModel;
using operations_research::RoutingSearchParameters;

struct DataModel {
  const std::vector<std::vector<int64_t>> distance_matrix = {
      {0, 9, 8, 7, 3, 6},
      {9, 0, 4, 2, 8, 7},
      {8, 4, 0, 6, 5, 3},
      {7, 2, 6, 0, 4, 6},
      {3, 8, 5, 4, 0, 7},
      {6, 7, 3, 6, 7, 0},
  };

  const std::vector<int64_t> demands = {0, 1, 1, 2, 4, 2};
  const int64_t vehicle_capacity = 5;

  const int num_vehicles = 2;
  const RoutingIndexManager::NodeIndex depot{0};
};

static void PrintSolution(const DataModel& data,
                          const RoutingIndexManager& manager,
                          const RoutingModel& routing,
                          const Assignment& solution) {
  int64_t total_distance = 0;
  int64_t total_load = 0;

  for (int vehicle_id = 0; vehicle_id < data.num_vehicles; ++vehicle_id) {
    int64_t index = routing.Start(vehicle_id);
    std::ostringstream route;
    int64_t route_distance = 0;
    int64_t route_load = 0;

    route << "Route for vehicle " << vehicle_id << ":\n";

    while (!routing.IsEnd(index)) {
      int node = manager.IndexToNode(index).value();
      route_load += data.demands[node];
      route << node << " Load(" << route_load << ") -> ";
      int64_t previous_index = index;
      index = solution.Value(routing.NextVar(index));
      route_distance += routing.GetArcCostForVehicle(previous_index, index, vehicle_id);
    }

    route << manager.IndexToNode(index).value() << " Load(" << route_load << ")\n";
    route << "Distance of the route: " << route_distance << "\n";
    route << "Load of the route: " << route_load << "\n\n";

    std::cout << route.str();
    total_distance += route_distance;
    total_load += route_load;
  }

  std::cout << "Total distance of all routes: " << total_distance << "\n";
  std::cout << "Total load of all routes: " << total_load << "\n";
  std::cout << "Solved in " << routing.solver()->wall_time() << "ms\n";
}

int main() {
  DataModel data;

  RoutingIndexManager manager(static_cast<int>(data.distance_matrix.size()),
                              data.num_vehicles, data.depot);
  RoutingModel routing(manager);

  const int transit_callback_index = routing.RegisterTransitCallback(
      [&data, &manager](int64_t from_index, int64_t to_index) -> int64_t {
        const int from_node = manager.IndexToNode(from_index).value();
        const int to_node = manager.IndexToNode(to_index).value();
        return data.distance_matrix[from_node][to_node];
      });

  routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);

  const int demand_callback_index = routing.RegisterUnaryTransitCallback(
      [&data, &manager](int64_t from_index) -> int64_t {
        const int from_node = manager.IndexToNode(from_index).value();
        return data.demands[from_node];
      });

  routing.AddDimensionWithVehicleCapacity(
      demand_callback_index,
      int64_t{0},
      std::vector<int64_t>(data.num_vehicles, data.vehicle_capacity),
      true,
      "Capacity");

  RoutingSearchParameters search_parameters =
      operations_research::DefaultRoutingSearchParameters();
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