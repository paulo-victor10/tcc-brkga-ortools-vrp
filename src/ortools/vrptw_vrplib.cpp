#include <cmath>
#include <iostream>
#include <vector>

#include "ortools/constraint_solver/routing.h"
#include "ortools/constraint_solver/routing_index_manager.h"
#include "ortools/constraint_solver/routing_parameters.h"

#include "parsers/vrplib_vrptw.h"

using operations_research::RoutingIndexManager;
using operations_research::RoutingModel;
using operations_research::RoutingSearchParameters;

static int64_t Euc2dRounded(double x1,double y1,double x2,double y2){
  double dx=x1-x2;
  double dy=y1-y2;
  return std::floor(std::sqrt(dx*dx+dy*dy)+0.5);
}

int main(int argc,char** argv){

  if(argc<4){
    std::cout<<"usage: vrptw_vrplib instance vehicles time\n";
    return 1;
  }

  std::string instance_path=argv[1];
  int vehicles=std::stoi(argv[2]);
  int time_limit=std::stoi(argv[3]);

  auto inst=LoadVRPLIB_VRPTW(instance_path);

  RoutingIndexManager manager(inst.dimension,vehicles,inst.depot);
  RoutingModel routing(manager);

  auto transit=routing.RegisterTransitCallback(
  [&](int64_t from,int64_t to){

    int f=manager.IndexToNode(from).value();
    int t=manager.IndexToNode(to).value();

    return Euc2dRounded(
      inst.x[f],inst.y[f],
      inst.x[t],inst.y[t]
    );

  });

  routing.SetArcCostEvaluatorOfAllVehicles(transit);

  auto demand=routing.RegisterUnaryTransitCallback(
  [&](int64_t from){

    int f=manager.IndexToNode(from).value();
    return inst.demand[f];

  });

  routing.AddDimensionWithVehicleCapacity(
    demand,
    0,
    std::vector<int64_t>(vehicles,inst.capacity),
    true,
    "Capacity"
  );

  routing.AddDimension(
    transit,
    30,
    100000,
    false,
    "Time"
  );

  auto& time_dimension=routing.GetDimensionOrDie("Time");

  for(int i=0;i<inst.dimension;i++){

    int64_t index=manager.NodeToIndex(i);

    time_dimension.CumulVar(index)->SetRange(
      inst.ready_time[i],
      inst.due_time[i]
    );
  }

  RoutingSearchParameters params=
    operations_research::DefaultRoutingSearchParameters();

  params.set_first_solution_strategy(
    operations_research::FirstSolutionStrategy::
    PARALLEL_CHEAPEST_INSERTION
  );

  params.set_local_search_metaheuristic(
    operations_research::LocalSearchMetaheuristic::
    GUIDED_LOCAL_SEARCH
  );

  params.mutable_time_limit()->set_seconds(time_limit);

  const auto* solution=routing.SolveWithParameters(params);

  if(!solution){
    std::cout<<"No solution\n";
    return 1;
  }

  int64_t cost=solution->ObjectiveValue();

  std::cout<<"Total distance: "<<cost<<"\n";
  std::cout<<"Solved in "<<routing.solver()->wall_time()<<"ms\n";

  return 0;
}