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

#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

// packages
#include <optional>
#include <string>
#include <vector>

// files
#include "simulation.hpp"

struct SimulationConfig {
	int numBombs = 1;
	int choice = 3;

	std::vector<ImpactScenario> scenarios;
	PhysicsConstants cons;
	AtmosphereState atmos;
	Aircraft bomber;
	Projectile munition;
	Target object;
};


// ! Class for loading and parsing simulation targets and projectiles configuration data
class ConfigLoader {
public:
	// Parse JSON input specifically for the AI/automation pipeline
	static SimulationConfig loadSimulationConfig(const std::string& filepath);
};

#endif // CONFIG_LOADER_HPP
