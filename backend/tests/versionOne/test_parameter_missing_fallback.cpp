#include <iostream>
#include <cassert>
#include <fstream>
#include "nlohmann/json.hpp"
#include "penetration/versionOne/simulation.hpp"
#include "penetration/versionOne/config_loader.hpp"

using json = nlohmann::json;

void runTest() {
    std::cout << "[TEST] Parameter Missing Fallback...\n";
    
    // Create malformed JSON payload (Missing Aircraft, AtmosphereState, Target layers)
    json j = {
        {"Simulation", {{"choice", 1}, {"numBombs", 1}}},
        {"Projectile", {
            {"name", "Test Bomb"} // Only providing name, rest should use fallbacks
        }},
        {"Target", {
            {"name", "Test Bunker"} // Missing layers entirely
        }},
        // Missing Aircraft entirely
        // Missing AtmosphereState entirely
        {"Scenario", {
            {"altitude_ft", 10000.0}
            // Missing velocity, fpa, obliq, aoa
        }}
    };
    
    std::ofstream o("temp_missing.json");
    o << j << std::endl;
    o.close();

    // Load via AI Path
    SimulationConfig aiConfig = ConfigLoader::loadSimulationConfig("temp_missing.json");

    // Asserts to ensure fallbacks were correctly triggered and program didn't crash
    assert(aiConfig.munition.total_mass == 14000.0); // Default mass fallback
    assert(aiConfig.object.layers.size() == 1); // Fallback layer created
    assert(aiConfig.object.layers[0].material_name == "AI Custom Layer (Fallback)");
    assert(aiConfig.bomber.name == "Unknown Aircraft");
    assert(aiConfig.atmos.speed_of_sound_ms == 340.3);
    assert(aiConfig.scenarios.size() == 1);
    assert(aiConfig.scenarios[0].flight_path_angle == 90.0); // Default FPA

    std::cout << "[TEST] Parameter Missing Fallback PASSED!\n";
}

int main() {
    runTest();
    return 0;
}
