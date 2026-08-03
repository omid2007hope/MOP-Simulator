// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include "default.hpp"
#include "simulation.hpp"
#include "config_loader.hpp"

bool approxEqual(double a, double b, double epsilon = 1e-4)
{
    return std::fabs(a - b) < epsilon;
}

int main()
{
    std::cout << "===================================================================================================\n";
    std::cout << "                              RUNNING SIMULATION UNIT TEST SUITE                                   \n";
    std::cout << "===================================================================================================\n";

    // Initialize test target and projectile using defaults
    Target concrete = CONCRETE_DEFAULT;
    Projectile mop = MOP_DEFAULT;

    PhysicsConstants cons;
    ImpactSimulator simulator(mop, concrete, cons);

    // Test 1: Subsonic Operational Impact (Mach ~1.0, 340 m/s)
    std::cout << "[Test 1] Testing Subsonic Rigid Penetration (340 m/s)...\n";
    ImpactScenario subScenario {"Subsonic Test", 0.0, 340.0, 0.0, 0.0};
    SimulationResult resSub = simulator.simulate(subScenario);

    assert(approxEqual(resSub.kinetic_energy, 786080000.0, 1.0));
    assert(resSub.casing_failure == false);
    assert(resSub.explosive_charge_survives == true);
    assert(resSub.regime == "Rigid Penetration (Crater+Tunnel)");
    
    // Check Crater + Tunnel continuity (Phase 2 to Phase 3)
    bool crater_phase_exists = false;
    bool tunnel_phase_exists = false;
    for (const auto& f : resSub.penetration_frames) {
        if (f.depth <= mop.diameter * 2.0) crater_phase_exists = true;
        if (f.depth > mop.diameter * 2.0) tunnel_phase_exists = true;
    }
    assert(crater_phase_exists && tunnel_phase_exists);
    assert(resSub.actual_penetration_depth > 3.5 && resSub.actual_penetration_depth < 4.5); // Output is 3.88m
    
    std::cout << "         -> [PASS] Kinetic Energy: " << (resSub.kinetic_energy / 1e9) << " GJ\n";
    std::cout << "         -> [PASS] Max Dynamic Pressure: " << (resSub.dynamic_pressure / 1e9) << " GPa\n";
    std::cout << "         -> [PASS] Casing remained intact in Rigid Penetration regime.\n";
    std::cout << "         -> [PASS] Crater/Tunnel Continuity confirmed.\n\n";

    // Test 2: Hypervelocity Impact (Mach ~14.7, 1500 m/s) - Tests Walker-Wasley Triggering
    std::cout << "[Test 2] Testing Hypervelocity Impact (1500 m/s) for Walker-Wasley Shock Initiation...\n";
    ImpactScenario hyperScenario {"Hypervelocity Test", 50000.0, 1500.0, 0.0, 0.0};
    SimulationResult resHyper = simulator.simulate(hyperScenario);
    
    assert(resHyper.regime == "Shock Initiation (Walker-Wasley)");
    assert(resHyper.casing_failure == true || !resHyper.explosive_charge_survives);
    assert(resHyper.shock_damage_prob_percent > 99.0);
    assert(resHyper.shock_pressure_gpa_peak > 1.0); // Should be very high (Hugoniot mismatch)
    
    std::cout << "         -> [PASS] Kinetic Energy: " << (resHyper.kinetic_energy / 1e9) << " GJ\n";
    std::cout << "         -> [PASS] Failure correctly detected. Regime: " << resHyper.regime << "\n";
    std::cout << "         -> [PASS] Peak Shock Pressure: " << resHyper.shock_pressure_gpa_peak << " GPa\n\n";

    // Test 3: Orbital Kinetic Strike ("Rods from God" Tungsten Rod, 3400 m/s) - Tests WAPM Erosion
    std::cout << "[Test 3] Testing Orbital Tungsten Kinetic Rod (3400 m/s) for WAPM Erosion...\n";
    Projectile rod = RODS_FROM_GOD_DEFAULT;
    ImpactSimulator rodSim(rod, concrete, cons);
    ImpactScenario rodScenario {"LEO Strike", 100000.0, 3400.0, 0.0, 0.0};
    SimulationResult resRod = rodSim.simulate(rodScenario);

    assert(resRod.is_kinetic_rod == true);
    assert(resRod.regime == "Hypervelocity Erosion Burnout");
    assert(resRod.erosion_occurred == true);
    assert(resRod.erosion_length_lost > 0.0);
    assert(resRod.final_rod_length < rod.length);
    assert(resRod.actual_penetration_depth > 16.0 && resRod.actual_penetration_depth < 18.0); // Output was 17.0m

    std::cout << "         -> [PASS] Identified as Kinetic Rod.\n";
    std::cout << "         -> [PASS] Regime: " << resRod.regime << "\n";
    std::cout << "         -> [PASS] WAPM Erosion Engagement confirmed. Lost length: " << resRod.erosion_length_lost << " m\n\n";

    // Test 4: Dynamic Increase Factor (DIF) Validation
    std::cout << "[Test 4] Testing Dynamic Increase Factor (DIF) strain-rate strengthening...\n";
    // Check if DIF was calculated correctly in Test 1
    bool dif_active = false;
    for (const auto& f : resSub.penetration_frames) {
        if (f.dif > 1.05) dif_active = true; // DIF should exceed 1.0 due to high strain rates
    }
    assert(dif_active);
    assert(resSub.dynamic_increase_factor > 1.0);
    
    std::cout << "         -> [PASS] DIF actively strengthened target dynamically.\n\n";

    // Test 5: Oblique Impact
    std::cout << "[Test 5] Testing Oblique Impact (Obliquity 30 deg, AoA 5 deg, 400 m/s)...\n";
    ImpactScenario obliqueScenario {"Oblique Test", 15.0, 400.0, 30.0, 5.0};
    SimulationResult resOblique = simulator.simulate(obliqueScenario);
    
    assert(resOblique.actual_penetration_depth > 5.0 && resOblique.actual_penetration_depth < 6.0); // 5.31m
    
    std::cout << "         -> [PASS] Oblique impact depth validated (Depth: " << resOblique.actual_penetration_depth << " m).\n\n";

    std::cout << "                      [PASS] ALL UNIT TESTS PASSED SUCCESSFULLY!                                   \n";
    std::cout << "===================================================================================================\n\n";

    return 0;
}
