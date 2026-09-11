// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "penetration/versionOne/config_loader.hpp"
#include "penetration/versionOne/simulation.hpp"

bool approxEqual(double a, double b, double epsilon = 1e-4) {
	return std::fabs(a - b) < epsilon;
}

int main() {
	std::cout
		<< "===================================================================================================\n";
	std::cout
		<< "                              RUNNING SIMULATION UNIT TEST SUITE                                   \n";
	std::cout
		<< "===================================================================================================\n";

	// Initialize test target and projectile using dummy setup instead of deleted default.hpp
	TargetLayer layer{"Concrete", 10.0, 0.03, 400e6, 2400.0, 50e6, 3000.0, 1.8, 850.0, 1600.0, 350000.0};
    Target concrete{"Test Bunker", {layer}};
    
    Projectile mop{"Test Bomb", 5.0, 0.5, 1.0, 10000.0, 2000.0, 5e6, 7800.0, 1.2e9, 0.015, 210e9, 0.04, 4500.0, 1.5, 2e15, 450.0, 1700.0, 270000.0};

	PhysicsConstants cons;
	AtmosphereState atmos;
	atmos.density_kgm3 = 0.01; // Low density for testing to prevent drag from slowing down hypervelocity strikes
	atmos.speed_of_sound_ms = 340.3;
	atmos.pressure_Pa = 101325.0;
	atmos.temperature_K = 288.15;
    
    Aircraft dummyAircraft{"Test Bomber", 150000.0, 450.0, 4.0};
	ImpactSimulator simulator(mop, concrete, cons);

	// Test 1: Subsonic Operational Impact (Mach ~1.0, 340 m/s)
	std::cout << "[Test 1] Testing Subsonic Rigid Penetration (340 m/s)...\n";
	ImpactScenario subScenario {"Subsonic Test", 0.0, 340.0, 90.0, 0.0, 0.0};
	SimulationResult resSub = simulator.simulate(subScenario, dummyAircraft, atmos);

	assert(resSub.casing_failure == false);
	assert(resSub.explosive_charge_survives == true);
	assert(resSub.regime == "Rigid Penetration (Crater+Tunnel)");

	// Check Crater + Tunnel continuity (Phase 2 to Phase 3)
	bool crater_phase_exists = false;
	bool tunnel_phase_exists = false;
	for (const auto& f : resSub.penetration_frames) {
		if (f.depth <= mop.diameter * 2.0)
			crater_phase_exists = true;
		if (f.depth > mop.diameter * 2.0)
			tunnel_phase_exists = true;
	}
	assert(crater_phase_exists && tunnel_phase_exists);

	std::cout << "         -> [PASS] Kinetic Energy: " << (resSub.kinetic_energy / 1e9)
		  << " GJ\n";
	std::cout << "         -> [PASS] Max Dynamic Pressure: " << (resSub.dynamic_pressure / 1e9)
		  << " GPa\n";
	std::cout << "         -> [PASS] Casing remained intact in Rigid Penetration regime.\n";
	std::cout << "         -> [PASS] Crater/Tunnel Continuity confirmed.\n\n";

	// Test 2: Hypervelocity Impact (Mach ~14.7, 5000 m/s) - Tests Walker-Wasley Triggering
	std::cout
		<< "[Test 2] Testing Hypervelocity Impact (5000 m/s) for Walker-Wasley Shock Initiation...\n";
	ImpactScenario hyperScenario {"Hypervelocity Test", 50000.0, 5000.0, 90.0, 0.0, 0.0};
	SimulationResult resHyper = simulator.simulate(hyperScenario, dummyAircraft, atmos);

	assert(resHyper.regime == "Shock Initiation (Walker-Wasley)");
	assert(resHyper.casing_failure == true || !resHyper.explosive_charge_survives);
	assert(resHyper.shock_damage_prob_percent > 99.0);
	assert(resHyper.shock_pressure_gpa_peak > 1.0); // Should be very high (Hugoniot mismatch)

	std::cout << "         -> [PASS] Kinetic Energy: " << (resHyper.kinetic_energy / 1e9)
		  << " GJ\n";
	std::cout << "         -> [PASS] Failure correctly detected. Regime: " << resHyper.regime
		  << "\n";
	std::cout << "         -> [PASS] Peak Shock Pressure: " << resHyper.shock_pressure_gpa_peak
		  << " GPa\n\n";

	// Test 3: Orbital Kinetic Strike ("Rods from God" Tungsten Rod, 3400 m/s) - Tests WAPM Erosion
	std::cout
		<< "[Test 3] Testing Orbital Tungsten Kinetic Rod (3400 m/s) for WAPM Erosion...\n";
	Projectile rod{"Rod", 6.0, 0.3, 1.0, 8000.0, 0.0, 0.0, 19250.0, 1.5e9, 0.015, 410e9, 0.15, 4000.0, 1.2, 0.0, 130.0, 3400.0, 350000.0};
	ImpactSimulator rodSim(rod, concrete, cons);
	ImpactScenario rodScenario {"LEO Strike", 100000.0, 3400.0, 90.0, 0.0, 0.0};
	SimulationResult resRod = rodSim.simulate(rodScenario, dummyAircraft, atmos);

	assert(resRod.is_kinetic_rod == true);
	assert(resRod.regime == "Hypervelocity Erosion Burnout");
	assert(resRod.erosion_occurred == true);
	assert(resRod.erosion_length_lost > 0.0);
	assert(resRod.final_rod_length < rod.length);
	
	std::cout << "         -> [PASS] Identified as Kinetic Rod.\n";
	std::cout << "         -> [PASS] Regime: " << resRod.regime << "\n";
	std::cout << "         -> [PASS] WAPM Erosion Engagement confirmed. Lost length: "
		  << resRod.erosion_length_lost << " m\n\n";

	// Test 4: Dynamic Increase Factor (DIF) Validation
	std::cout
		<< "[Test 4] Testing Dynamic Increase Factor (DIF) strain-rate strengthening...\n";
	// Check if DIF was calculated correctly in Test 1
	bool dif_active = false;
	for (const auto& f : resSub.penetration_frames) {
		if (f.dif > 1.05)
			dif_active = true; // DIF should exceed 1.0 due to high strain rates
	}
	assert(dif_active);
	assert(resSub.dynamic_increase_factor > 1.0);

	std::cout << "         -> [PASS] DIF actively strengthened target dynamically.\n\n";

	// Test 5: Oblique Impact
	std::cout << "[Test 5] Testing Oblique Impact (Obliquity 30 deg, AoA 5 deg, 400 m/s)...\n";
	ImpactScenario obliqueScenario {"Oblique Test", 15.0, 400.0, 90.0, 30.0, 5.0};
	SimulationResult resOblique = simulator.simulate(obliqueScenario, dummyAircraft, atmos);

	assert(resOblique.casing_failure == false);
	assert(resOblique.regime == "Rigid Penetration (Crater+Tunnel)");

	std::cout << "         -> [PASS] Oblique impact successfully survived without structural failure (J-Hook fixed) at Depth: "
		  << resOblique.actual_penetration_depth << " m.\n\n";

	std::cout
		<< "                      [PASS] ALL UNIT TESTS PASSED SUCCESSFULLY!                                   \n";
	std::cout
		<< "===================================================================================================\n\n";

	return 0;
}
