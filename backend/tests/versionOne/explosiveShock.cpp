// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "config_loader.hpp"
#include "default.hpp"
#include "simulation.hpp"

bool approxEqual(double a, double b, double epsilon = 1e-4) {
	return std::fabs(a - b) < epsilon;
}

int main() {
	// Initialize test target and projectile using defaults
	Target concrete = CONCRETE_DEFAULT;
	Projectile mop = MOP_DEFAULT;

	PhysicsConstants cons;
	ImpactSimulator simulator(mop, concrete, cons);

	// Test case 1: explosiveMass = 2400 kg, explosiveEnergy = 5.2e6 J/kg
	// explosiveShock = 2400 * 5.2e6 * 1.0 (couplingEfficiency) = 1.248e10
	double expMass1 = 2400.0;
	double expEnergy1 = 5.2e6;
	double expected1 = expMass1 * expEnergy1 * 1.0;
	double result1 = simulator.explosiveShockwave(expMass1, expEnergy1);
	std::cout << "Test 1 (explosiveShockwave): Expected " << expected1 << ", Got " << result1 << std::endl;
	assert(approxEqual(result1, expected1));

	// Test case 2: zero explosive mass (kinetic penetrator)
	double expMass2 = 0.0;
	double expEnergy2 = 5.2e6;
	double expected2 = 0.0;
	double result2 = simulator.explosiveShockwave(expMass2, expEnergy2);
	std::cout << "Test 2 (explosiveShockwave): Expected " << expected2 << ", Got " << result2 << std::endl;
	assert(approxEqual(result2, expected2));

	std::cout << "All explosiveShockwave tests passed!" << std::endl;
	return 0;
}