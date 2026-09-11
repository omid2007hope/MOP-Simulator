// Copyright (c) 2026 Omid Teimory. All Rights Reserved

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
