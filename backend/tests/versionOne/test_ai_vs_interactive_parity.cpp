#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>
#include "nlohmann/json.hpp"
#include "penetration/versionOne/simulation.hpp"
#include "penetration/versionOne/config_loader.hpp"
#include "penetration/versionOne/telemetry_exporter.hpp"

using json = nlohmann::json;

void runTest() {
    std::cout << "[TEST] AI vs Interactive Parity...\n";
    
    // 1. Create JSON Payload
    json j = {
        {"Simulation", {{"choice", 1}, {"numBombs", 1}}},
        {"Projectile", {
            {"name", "Test Bomb"}, {"length", 5.0}, {"diameter", 0.5},
            {"curvature_noseReduce", 1.0}, {"total_mass", 10000.0},
            {"explosive_mass", 2000.0}, {"explosive_energy_j_per_kg", 5e6},
            {"casing_density", 7800.0}, {"yield_strength", 1.2e9},
            {"area_moment_inertia", 0.015}, {"elastic_modulus", 210e9},
            {"casing_wall_thickness", 0.04}, {"hugoniot_c0", 4500.0},
            {"hugoniot_s", 1.5}, {"explosive_critical_energy", 2e15},
            {"specific_heat", 450.0}, {"melting_point", 1700.0},
            {"heat_of_fusion", 270000.0}
        }},
        {"Target", {
            {"name", "Test Bunker"},
            {"layers", {{
                {"material_name", "Concrete"}, {"thickness", 10.0},
                {"rebar_volume_fraction", 0.03}, {"rebar_yield_strength", 400e6},
                {"density", 2400.0}, {"compressive_strength", 50e6},
                {"hugoniot_c0", 3000.0}, {"hugoniot_s", 1.8},
                {"specific_heat", 850.0}, {"melting_point", 1600.0},
                {"heat_of_fusion", 350000.0}
            }}}
        }},
        {"Aircraft", {
            {"name", "Test Bomber"}, {"bomber_totalMass", 150000.0},
            {"bomber_wingArea", 450.0}, {"bomber_liftCurveSlope", 4.0}
        }},
        {"AtmosphereState", {
            {"temperature_K", 300.0}, {"pressure_Pa", 100000.0},
            {"density_kgm3", 1.2}, {"speed_of_sound_ms", 340.0}
        }},
        {"Scenario", {
            {"altitude_ft", 30000.0}, {"velocity", 0.0},
            {"flight_path_angle", 90.0}, {"obliquity_angle", 0.0},
            {"angle_of_attack", 0.0}
        }}
    };
    
    std::ofstream o("temp_test.json");
    o << j << std::endl;
    o.close();

    SimulationConfig aiConfig = ConfigLoader::loadSimulationConfig("temp_test.json");

    Projectile manualProj;
    manualProj.name = "Test Bomb";
    manualProj.length = 5.0; manualProj.diameter = 0.5;
    manualProj.curvature_noseReduce = 1.0; manualProj.total_mass = 10000.0;
    manualProj.explosive_mass = 2000.0; manualProj.explosive_energy_j_per_kg = 5e6;
    manualProj.casing_density = 7800.0; manualProj.yield_strength = 1.2e9;
    manualProj.area_moment_inertia = 0.015; manualProj.elastic_modulus = 210e9;
    manualProj.casing_wall_thickness = 0.04; manualProj.hugoniot_c0 = 4500.0;
    manualProj.hugoniot_s = 1.5; manualProj.explosive_critical_energy = 2e15;
    manualProj.specific_heat = 450.0; manualProj.melting_point = 1700.0;
    manualProj.heat_of_fusion = 270000.0;

    Target manualTarg;
    manualTarg.name = "Test Bunker";
    TargetLayer layer;
    layer.material_name = "Concrete"; layer.thickness = 10.0;
    layer.rebar_volume_fraction = 0.03; layer.rebar_yield_strength = 400e6;
    layer.density = 2400.0; layer.compressive_strength = 50e6;
    layer.hugoniot_c0 = 3000.0; layer.hugoniot_s = 1.8;
    layer.specific_heat = 850.0; layer.melting_point = 1600.0;
    layer.heat_of_fusion = 350000.0;
    manualTarg.layers.push_back(layer);

    Aircraft manualAircraft{"Test Bomber", 150000.0, 450.0, 4.0};
    AtmosphereState manualAtmos{300.0, 100000.0, 1.2, 340.0};
    ImpactScenario manualScen{"Custom Test #1 (30000 ft drop)", 30000.0, 0.0, 90.0, 0.0, 0.0};

    PhysicsConstants cons;

    ImpactSimulator aiSimulator(aiConfig.munition, aiConfig.object, cons);
    SimulationResult aiResult = aiSimulator.simulate(aiConfig.scenarios[0], aiConfig.bomber, aiConfig.atmos);

    ImpactSimulator manualSimulator(manualProj, manualTarg, cons);
    SimulationResult manualResult = manualSimulator.simulate(manualScen, manualAircraft, manualAtmos);

    assert(std::abs(aiResult.actual_penetration_depth - manualResult.actual_penetration_depth) < 1e-6);
    assert(std::abs(aiResult.impact_velocity - manualResult.impact_velocity) < 1e-6);
    assert(std::abs(aiResult.shock_pressure_gpa_peak - manualResult.shock_pressure_gpa_peak) < 1e-6);
    assert(aiResult.casing_failure == manualResult.casing_failure);

    std::cout << "[TEST] AI vs Interactive Parity PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
