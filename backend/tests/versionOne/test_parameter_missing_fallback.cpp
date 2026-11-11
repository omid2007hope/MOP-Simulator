// SPDX-License-Identifier: LicenseRef-MopSimulator-Proprietary
/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * PROPRIETARY SOURCE-AVAILABLE SOFTWARE — NOT OPEN SOURCE.
 * Permission to view, clone, or compile this file is granted strictly on the
 * condition precedent of full compliance with the root ./LICENSE file.
 * Copying snippets, modifying, creating derivative works, redistributing,
 * commercial/SaaS use, and AI/ML training (reserved under EU Directive
 * 2019/790 Art. 4(3) and § 42h(6) Austrian UrhG) are strictly prohibited.
 * Removal or alteration of this notice violates 17 U.S.C. § 1202 and § 90c UrhG.
 */

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
