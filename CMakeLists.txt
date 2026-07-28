# // Copyright (c) 2026 Omid Teimory. All Rights Reserved

cmake_minimum_required(VERSION 3.15)
project(MOPSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set runtime output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Create the simulation core library
add_library(mop_sim_lib
    src/simulation.cpp
)

target_include_directories(mop_sim_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create the main executable
add_executable(sim
    src/main.cpp
)

target_link_libraries(sim PRIVATE mop_sim_lib)

# Enable testing and create the unit test executable
enable_testing()

add_executable(test_simulation
    tests/test_simulation.cpp
)

target_link_libraries(test_simulation PRIVATE mop_sim_lib)

add_test(NAME SimulationUnitTests COMMAND test_simulation)
