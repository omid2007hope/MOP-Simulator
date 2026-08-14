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
    src/simulation/penetration/simulation.cpp
    src/simulation/penetration/config_loader.cpp
    src/simulation/penetration/environment_physics.cpp
    src/simulation/penetration/telemetry_exporter.cpp
)

target_include_directories(mop_sim_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/penetration>
    $<INSTALL_INTERFACE:include>
)

# Create the main executable
add_executable(sim
    src/simulation/penetration/main.cpp
)

target_link_libraries(sim PRIVATE mop_sim_lib)

# Enable testing and create the unit test executable
enable_testing()

add_executable(test_simulation
    tests/test_simulation.cpp
)
target_link_libraries(test_simulation PRIVATE mop_sim_lib)
add_test(NAME SimulationUnitTests COMMAND test_simulation)

add_executable(test_impactShock
    tests/impactShock.cpp
)
target_link_libraries(test_impactShock PRIVATE mop_sim_lib)
add_test(NAME ImpactShockTests COMMAND test_impactShock)

add_executable(test_explosiveShock
    tests/explosiveShock.cpp
)
target_link_libraries(test_explosiveShock PRIVATE mop_sim_lib)
add_test(NAME ExplosiveShockTests COMMAND test_explosiveShock)
