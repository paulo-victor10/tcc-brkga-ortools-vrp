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

# --- Configuração do PoC QAP com OR-Tools ---

# Define o executável e aponta para o arquivo que criamos
add_executable(ortools_qap_poc src/qap/ortools_qap_poc.cpp)

# Linka as bibliotecas do OR-Tools (CP-SAT)
target_link_libraries(ortools_qap_poc PRIVATE ortools::ortools)

# --- Configuração do BRKGA QAP ---

# Adiciona o executável do BRKGA QAP
add_executable(brkga_qap 
    src/qap/brkga_qap_main.cpp 
    src/qap/QapDecoder.cpp
)

# Diz ao compilador onde encontrar os arquivos .hpp do brkga-cuda
target_include_directories(brkga_qap PRIVATE 
    src 
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/brkga-cuda/src
)

# Exigência do CMake para alvos CUDA: define a arquitetura da placa de vídeo
set_property(TARGET brkga_qap PROPERTY CUDA_ARCHITECTURES 61)

# Linka com a biblioteca brkga-cuda compilada pela GPU
target_link_libraries(brkga_qap PRIVATE brkga-cuda)