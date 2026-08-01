#include "config_loader.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::vector<Target> ConfigLoader::loadTargets(const std::string& filepath)
{
    std::vector<Target> targets;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << filepath << ". Using defaults.\n";
        return targets;
    }

    try {
        json j;
        file >> j;
        for (const auto& item : j) {
            Target t;
            t.name = item.value("name", "Unknown Target");
            t.density = item.value("density", 2500.0);
            t.compressiveStrength = item.value("compressive_strength", 60.0e6);
            targets.push_back(t);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON in " << filepath << ": " << e.what() << "\n";
    }

    return targets;
}

std::vector<Projectile> ConfigLoader::loadProjectiles(const std::string& filepath)
{
    std::vector<Projectile> projectiles;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << filepath << ". Using defaults.\n";
        return projectiles;
    }

    try {
        json j;
        file >> j;
        for (const auto& item : j) {
            Projectile p;
            p.name = item.value("name", "Unknown Projectile");
            p.length = item.value("length", 1.0);
            p.diameter = item.value("diameter", 0.1);
            p.total_mass = item.value("total_mass", 100.0);
            p.explosive_mass = item.value("explosive_mass", 0.0);
            p.casing_density = item.value("casing_density", 7800.0);
            p.yield_strength = item.value("yield_strength", 1.0e9);
            projectiles.push_back(p);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON in " << filepath << ": " << e.what() << "\n";
    }

    return projectiles;
}

std::optional<Target> ConfigLoader::getTargetByName(const std::vector<Target>& targets, const std::string& name)
{
    for (const auto& t : targets) {
        if (t.name == name) {
            return t;
        }
    }
    return std::nullopt;
}

std::optional<Projectile> ConfigLoader::getProjectileByName(const std::vector<Projectile>& projectiles, const std::string& name)
{
    for (const auto& p : projectiles) {
        if (p.name == name) {
            return p;
        }
    }
    return std::nullopt;
}
