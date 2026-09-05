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
	static std::vector<ImpactScenario> loadImpactScenario(const std::string& filepath);

	static std::vector<PhysicsConstants> loadPhysicsConstants(const std::string& filepath);

	static std::vector<AtmosphereState> loadAtmosphereState(const std::string& filepath);

	static std::vector<Aircraft> loadAircraft(const std::string& filepath);

	// Load projectiles from a JSON file
	static std::vector<Projectile> loadProjectiles(const std::string& filepath);

	// Load targets from a JSON file
	static std::vector<Target> loadTargets(const std::string& filepath);

	// ************
	// By name
	// ************

	// Find a target by name (returns std::nullopt if not found)
	static std::optional<Target> getTargetByName(const std::vector<Target>& targets,
						     const std::string& name);

	// Find a projectile by name (returns std::nullopt if not found)
	static std::optional<Projectile> getProjectileByName(
		const std::vector<Projectile>& projectiles, const std::string& name);

	// Find an impact scenario by name (returns std::nullopt if not found)
	static std::optional<ImpactScenario> getScenarioByName(
		const std::vector<ImpactScenario>& scenarios, const std::string& name);

	// Find an atmosphere state by name index (returns std::nullopt if out of range)
	static std::optional<AtmosphereState> getAtmosphereStateByIndex(
		const std::vector<AtmosphereState>& states, int index);

	// Find an aircraft by name (returns std::nullopt if not found)
	static std::optional<Aircraft> getAircraftByName(
		const std::vector<Aircraft>& aircraft, const std::string& name);

	// Parse JSON input specifically for the AI/automation pipeline
	static SimulationConfig loadSimulationConfig(const std::string& filepath,
						     const std::vector<Projectile>& projectilesDb);
};

#endif // CONFIG_LOADER_HPP
