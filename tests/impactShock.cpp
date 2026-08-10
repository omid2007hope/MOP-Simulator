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

	// Test case 1: totalMass = 13600 kg, velocity = 500 m/s
	// kineticShock = 0.5 * 13600 * 500^2 = 1.7e9 Joules
	double mass1 = 13600.0;
	double vel1 = 500.0;
	double expected1 = 0.5 * mass1 * std::pow(vel1, 2);
	double result1 = ImpactSimulator::impactShockwave(mass1, vel1);
	std::cout << "Test 1 (impactShockwave): Expected " << expected1 << ", Got " << result1 << std::endl;
	assert(approxEqual(result1, expected1));

	// Test case 2: zero velocity
	double mass2 = 1000.0;
	double vel2 = 0.0;
	double expected2 = 0.0;
	double result2 = ImpactSimulator::impactShockwave(mass2, vel2);
	std::cout << "Test 2 (impactShockwave): Expected " << expected2 << ", Got " << result2 << std::endl;
	assert(approxEqual(result2, expected2));

	std::cout << "All impactShockwave tests passed!" << std::endl;
	return 0;
}