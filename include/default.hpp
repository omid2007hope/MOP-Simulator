#ifndef DEFAULT_HPP
#define DEFAULT_HPP

#include <string>
#include <vector>
#include "simulation.hpp"

struct MOP_DEFAULT
{
    const std::string default_name = "GBU-57 Massive Ordnance Penetrator (MOP)";
    const double default_length = 6.2;
    const double default_diameter = 0.8;
    const double default_total_mass = 13600.0;
    const double default_explosive_mass = 2400.0;
    const double default_casing_density = 7800.0;
    const double default_yield_strength = 2.0e9; // 2.0 GPa
    const double default_specific_heat = 460.0; // J/(kg*K) for steel
    const double default_melting_point = 1800.0; // Kelvin
    const double default_heat_of_fusion = 272000.0; // J/kg
    const double default_area_moment_inertia = 0.02; // m^4 (approximate)
};

struct CONCRETE_DEFAULT
{
    const std::string default_name = "High-Quality Hardened Concrete Structure";
    const std::vector<TargetLayer> default_layers = {
        {"Soil Overburden", 2.0, 1500.0, 10.0e6, 0.0, 0.0},
        {"Heavily Reinforced Concrete", 5.0, 2650.0, 60.0e6, 0.02, 400.0e6},
        {"Solid Granite Rock", 50.0, 2750.0, 130.0e6, 0.0, 0.0}
    };
};

struct RODS_FROM_GOD_DEFAULT
{
    const std::string default_name = "Orbital Tungsten Kinetic Penetrator (Rods from God)";
    const double default_length = 6.1;
    const double default_diameter = 0.3;
    const double default_total_mass = 8300.0;
    const double default_explosive_mass = 0.0;     // 0 kg explosive
    const double default_casing_density = 19300.0; // High-density Tungsten
    const double default_yield_strength = 0.0;     // 0 GPa
    const double default_specific_heat = 134.0;    // J/(kg*K) for tungsten
    const double default_melting_point = 3695.0;   // Kelvin
    const double default_heat_of_fusion = 192000.0; // J/kg
    const double default_area_moment_inertia = 0.0004; // m^4
};

#endif // DEFAULT_HPP
