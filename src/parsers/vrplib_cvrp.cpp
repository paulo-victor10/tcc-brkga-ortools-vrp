#include "vrplib_cvrp.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

static std::string Trim(const std::string& s) {
  size_t b = 0;
  while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
  size_t e = s.size();
  while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
  return s.substr(b, e - b);
}

static bool StartsWith(const std::string& s, const std::string& prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static int ParseIntAfterColon(const std::string& line) {
  const auto pos = line.find(':');
  if (pos == std::string::npos) throw std::runtime_error("Missing ':' in line: " + line);
  return std::stoi(Trim(line.substr(pos + 1)));
}

CVRPInstance LoadVRPLIB_CVRP(const std::string& filename) {
  std::ifstream file(filename);
  if (!file) throw std::runtime_error("Cannot open instance file: " + filename);

  CVRPInstance inst{};
  inst.dimension = -1;
  inst.capacity = -1;
  inst.depot = 0;

  enum class Section { None, NodeCoord, Demand, Depot };
  Section section = Section::None;

  int coords_read = 0;
  int demands_read = 0;

  std::string line;
  while (std::getline(file, line)) {
    line = Trim(line);
    if (line.empty()) continue;

    if (line == "NODE_COORD_SECTION") {
      section = Section::NodeCoord;
      continue;
    }
    if (line == "DEMAND_SECTION") {
      section = Section::Demand;
      continue;
    }
    if (line == "DEPOT_SECTION") {
      section = Section::Depot;
      continue;
    }
    if (line == "EOF") break;

    if (section == Section::None) {
      if (StartsWith(line, "DIMENSION")) {
        inst.dimension = ParseIntAfterColon(line);
        if (inst.dimension <= 0) throw std::runtime_error("Invalid DIMENSION");
        inst.x.assign(inst.dimension, 0.0);
        inst.y.assign(inst.dimension, 0.0);
        inst.demand.assign(inst.dimension, 0);
        continue;
      }
      if (StartsWith(line, "CAPACITY")) {
        inst.capacity = ParseIntAfterColon(line);
        if (inst.capacity <= 0) throw std::runtime_error("Invalid CAPACITY");
        continue;
      }
      continue;
    }

    if (section == Section::NodeCoord) {
      if (inst.dimension <= 0) throw std::runtime_error("DIMENSION must appear before NODE_COORD_SECTION");
      std::istringstream iss(line);
      int id = 0;
      double x = 0.0, y = 0.0;
      if (!(iss >> id >> x >> y)) continue;
      if (id < 1 || id > inst.dimension) throw std::runtime_error("NODE_COORD id out of range");
      inst.x[id - 1] = x;
      inst.y[id - 1] = y;
      coords_read++;
      if (coords_read >= inst.dimension) section = Section::None;
      continue;
    }

    if (section == Section::Demand) {
      if (inst.dimension <= 0) throw std::runtime_error("DIMENSION must appear before DEMAND_SECTION");
      std::istringstream iss(line);
      int id = 0;
      int dem = 0;
      if (!(iss >> id >> dem)) continue;
      if (id < 1 || id > inst.dimension) throw std::runtime_error("DEMAND id out of range");
      inst.demand[id - 1] = dem;
      demands_read++;
      if (demands_read >= inst.dimension) section = Section::None;
      continue;
    }

    if (section == Section::Depot) {
      std::istringstream iss(line);
      int depot_id = 0;
      if (!(iss >> depot_id)) continue;
      if (depot_id == -1) {
        section = Section::None;
        continue;
      }
      if (depot_id < 1 || depot_id > inst.dimension) throw std::runtime_error("DEPOT id out of range");
      inst.depot = depot_id - 1;
      continue;
    }
  }

  if (inst.dimension <= 0) throw std::runtime_error("Missing DIMENSION");
  if (inst.capacity <= 0) throw std::runtime_error("Missing CAPACITY");
  if (coords_read != inst.dimension) throw std::runtime_error("Incomplete NODE_COORD_SECTION");
  if (demands_read != inst.dimension) throw std::runtime_error("Incomplete DEMAND_SECTION");
  if (inst.depot < 0 || inst.depot >= inst.dimension) throw std::runtime_error("Invalid DEPOT");

  return inst;
}