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
    src/simulation/penetration/versionOne/simulation.cpp
    src/simulation/penetration/versionOne/config_loader.cpp
    src/simulation/penetration/versionOne/environment_physics.cpp
    src/simulation/penetration/versionOne/telemetry_exporter.cpp
)

target_include_directories(mop_sim_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/penetration/versionOne>
    $<INSTALL_INTERFACE:include>
)

# Create the main executable
add_executable(sim
    src/simulation/penetration/versionOne/main.cpp
)

target_link_libraries(sim PRIVATE mop_sim_lib)

# Enable testing and create the unit test executable
enable_testing()

add_executable(test_simulation
    tests/versionOne/test_simulation.cpp
)
target_link_libraries(test_simulation PRIVATE mop_sim_lib)
add_test(NAME SimulationUnitTests COMMAND test_simulation)

add_executable(test_impactShock
    tests/versionOne/impactShock.cpp
)
target_link_libraries(test_impactShock PRIVATE mop_sim_lib)
add_test(NAME ImpactShockTests COMMAND test_impactShock)

add_executable(test_explosiveShock
    tests/versionOne/explosiveShock.cpp
)
target_link_libraries(test_explosiveShock PRIVATE mop_sim_lib)
add_test(NAME ExplosiveShockTests COMMAND test_explosiveShock)

add_executable(test_aerodynamic_accuracy tests/versionOne/test_aerodynamic_accuracy.cpp)
target_link_libraries(test_aerodynamic_accuracy PRIVATE mop_sim_lib)
add_test(NAME AerodynamicAccuracyTests COMMAND test_aerodynamic_accuracy)

add_executable(test_ai_vs_interactive_parity tests/versionOne/test_ai_vs_interactive_parity.cpp)
target_link_libraries(test_ai_vs_interactive_parity PRIVATE mop_sim_lib)
add_test(NAME AiVsInteractiveParityTests COMMAND test_ai_vs_interactive_parity)

add_executable(test_parameter_missing_fallback tests/versionOne/test_parameter_missing_fallback.cpp)
target_link_libraries(test_parameter_missing_fallback PRIVATE mop_sim_lib)
add_test(NAME ParameterMissingFallbackTests COMMAND test_parameter_missing_fallback)

add_executable(test_penetration_accuracy tests/versionOne/test_penetration_accuracy.cpp)
target_link_libraries(test_penetration_accuracy PRIVATE mop_sim_lib)
add_test(NAME PenetrationAccuracyTests COMMAND test_penetration_accuracy)

add_executable(test_scenario_duplication tests/versionOne/test_scenario_duplication.cpp)
target_link_libraries(test_scenario_duplication PRIVATE mop_sim_lib)
add_test(NAME ScenarioDuplicationTests COMMAND test_scenario_duplication)

add_executable(test_stress_performance tests/versionOne/test_stress_performance.cpp)
target_link_libraries(test_stress_performance PRIVATE mop_sim_lib)
add_test(NAME StressPerformanceTests COMMAND test_stress_performance)

add_executable(test_telemetry_render_validation tests/versionOne/test_telemetry_render_validation.cpp)
target_link_libraries(test_telemetry_render_validation PRIVATE mop_sim_lib)
add_test(NAME TelemetryRenderValidationTests COMMAND test_telemetry_render_validation)
