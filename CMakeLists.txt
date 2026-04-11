cmake_minimum_required(VERSION 3.16)

project(
    tcc_brkga_ortools_vrp
    VERSION 0.1
    LANGUAGES CXX CUDA
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

add_executable(vrptw_vrplib
    src/ortools/vrptw_vrplib.cpp
    src/parsers/vrplib_vrptw.cpp
)
target_include_directories(vrptw_vrplib PRIVATE src)
target_link_libraries(vrptw_vrplib PRIVATE ortools::ortools)

add_subdirectory(brkga-cuda/src)

add_executable(cvrp_brkga 
    src/brkga/cvrp_brkga_main.cpp 
    src/brkga/CvrpDecoder.cpp 
    src/parsers/vrplib_cvrp.cpp
)

target_include_directories(cvrp_brkga PRIVATE src ${CMAKE_CURRENT_SOURCE_DIR}/brkga-cuda/src)

set_property(TARGET cvrp_brkga PROPERTY CUDA_ARCHITECTURES 61)

target_link_libraries(cvrp_brkga PRIVATE brkga-cuda)