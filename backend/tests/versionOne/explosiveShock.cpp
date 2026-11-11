/*
 * Copyright (c) 2026 Omid Teimory. All Rights Reserved.
 *
 * NOTICE: This file contains proprietary source code licensed exclusively under the
 * terms of the LICENSE file in the root directory of this repository.
 * You may view, download, and compile this code solely to run the unmodified product.
 * Copying, pasting, modifying, redistributing, creating derivative works, or using
 * this code in any other project or AI training system is strictly prohibited.
 * Removal or alteration of this notice is a violation of 17 U.S.C. § 1202 and EU law.
 */

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "penetration/versionOne/simulation.hpp"

bool approxEqual(double a, double b, double epsilon = 1e-4) {
	return std::fabs(a - b) < epsilon;
}

int main() {
	// Test case 1: explosiveMass = 2400 kg, explosiveEnergy = 5.2e6 J/kg
	// couplingEfficiency = 0.25 (empirical acoustic impedance coupling, documented in simulation.cpp)
	// explosiveShock = 2400 * 5.2e6 * 0.25 = 3.12e9
	double expMass1 = 2400.0;
	double expEnergy1 = 5.2e6;
	double expected1 = expMass1 * expEnergy1 * 0.25;
	double result1 = ImpactSimulator::explosiveShockwave(expMass1, expEnergy1);
	std::cout << "Test 1 (explosiveShockwave): Expected " << expected1 << ", Got " << result1 << std::endl;
	assert(approxEqual(result1, expected1));

	// Test case 2: zero explosive mass (kinetic penetrator)
	double expMass2 = 0.0;
	double expEnergy2 = 5.2e6;
	double expected2 = 0.0;
	double result2 = ImpactSimulator::explosiveShockwave(expMass2, expEnergy2);
	std::cout << "Test 2 (explosiveShockwave): Expected " << expected2 << ", Got " << result2 << std::endl;
	assert(approxEqual(result2, expected2));

	std::cout << "All explosiveShockwave tests passed!" << std::endl;
	return 0;
}