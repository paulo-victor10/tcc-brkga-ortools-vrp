#pragma once
#include <string>
#include <vector>

struct CVRPInstance {
  int dimension;
  int capacity;
  int depot;
  std::vector<double> x;
  std::vector<double> y;
  std::vector<int> demand;
};

CVRPInstance LoadVRPLIB_CVRP(const std::string& filename);