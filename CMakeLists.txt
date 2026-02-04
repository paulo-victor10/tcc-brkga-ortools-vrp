cmake_minimum_required(VERSION 3.16)

project(
    tcc_brkga_ortools_vrp
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ortools CONFIG REQUIRED)

add_executable(vrptw_demo src/ortools/vrptw_demo.cpp)
target_link_libraries(vrptw_demo PRIVATE ortools::ortools)

add_executable(cvrp_demo src/ortools/cvrp_demo.cpp)
target_link_libraries(cvrp_demo PRIVATE ortools::ortools)


add_executable(hello src/main.cpp)


add_executable(cvrp_vrplib src/ortools/cvrp_vrplib.cpp src/parsers/vrplib_cvrp.cpp)
target_include_directories(cvrp_vrplib PRIVATE src)
target_link_libraries(cvrp_vrplib PRIVATE ortools::ortools)