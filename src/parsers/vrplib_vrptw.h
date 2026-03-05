#pragma once

#include <string>
#include <vector>

struct VRPTWInstance {

  int dimension = 0;
  int capacity = 0;
  int depot = 0;

  std::vector<double> x;
  std::vector<double> y;

  std::vector<int64_t> demand;

  std::vector<int64_t> ready_time;
  std::vector<int64_t> due_time;
  std::vector<int64_t> service_time;

};

VRPTWInstance LoadVRPLIB_VRPTW(const std::string& path);