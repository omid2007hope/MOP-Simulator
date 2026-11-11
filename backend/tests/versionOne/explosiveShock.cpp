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