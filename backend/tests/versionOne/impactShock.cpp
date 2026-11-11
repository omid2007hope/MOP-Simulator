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