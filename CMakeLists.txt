# // Copyright (c) 2026 Omid Teimory. All Rights Reserved

cmake_minimum_required(VERSION 3.15)
project(MOPSimulator VERSION 2.8.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set runtime output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Create the simulation core library
add_library(mop_sim_lib
    src/simulation/simulation.cpp
    src/simulation/config_loader.cpp
)

target_include_directories(mop_sim_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create the main executable
add_executable(sim
    src/simulation/main.cpp
)

target_link_libraries(sim PRIVATE mop_sim_lib)

# Enable testing and create the unit test executable
enable_testing()

add_executable(test_simulation
    tests/test_simulation.cpp
)

target_link_libraries(test_simulation PRIVATE mop_sim_lib)

add_test(NAME SimulationUnitTests COMMAND test_simulation)
