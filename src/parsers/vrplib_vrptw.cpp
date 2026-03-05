#include "vrplib_vrptw.h"

#include <fstream>
#include <sstream>
#include <iostream>

VRPTWInstance LoadVRPLIB_VRPTW(const std::string& path) {

  VRPTWInstance inst;

  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Cannot open VRPTW file");
  }

  std::string line;

  while (std::getline(file, line)) {

    if (line.find("DIMENSION") != std::string::npos) {
      std::stringstream ss(line);
      std::string tmp;
      ss >> tmp >> tmp >> inst.dimension;
    }

    if (line.find("CAPACITY") != std::string::npos) {
      std::stringstream ss(line);
      std::string tmp;
      ss >> tmp >> tmp >> inst.capacity;
    }

    if (line.find("NODE_COORD_SECTION") != std::string::npos) {

      inst.x.resize(inst.dimension);
      inst.y.resize(inst.dimension);

      for (int i = 0; i < inst.dimension; i++) {
        int id;
        file >> id >> inst.x[i] >> inst.y[i];
      }
    }

    if (line.find("DEMAND_SECTION") != std::string::npos) {

      inst.demand.resize(inst.dimension);

      for (int i = 0; i < inst.dimension; i++) {
        int id;
        file >> id >> inst.demand[i];
      }
    }

    if (line.find("TIME_WINDOW_SECTION") != std::string::npos) {

      inst.ready_time.resize(inst.dimension);
      inst.due_time.resize(inst.dimension);

      for (int i = 0; i < inst.dimension; i++) {
        int id;
        file >> id >> inst.ready_time[i] >> inst.due_time[i];
      }
    }

    if (line.find("SERVICE_TIME_SECTION") != std::string::npos) {

      inst.service_time.resize(inst.dimension);

      for (int i = 0; i < inst.dimension; i++) {
        int id;
        file >> id >> inst.service_time[i];
      }
    }
  }

  return inst;
}