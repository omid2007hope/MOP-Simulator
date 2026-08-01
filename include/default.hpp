#ifndef DEFAULT_HPP
#define DEFAULT_HPP

#include <string>

struct MOP_DEFAULT
{
    const std::string default_name = "GBU-57 Massive Ordnance Penetrator (MOP)";
    const double default_length = 6.2;
    const double default_diameter = 0.8;
    const double default_total_mass = 13600.0;
    const double default_explosive_mass = 2400.0;
    const double default_casing_density = 7800.0;
    const double default_yield_strength = 2.0e9; // 2.0 GPa
};

struct CONCRETE_DEFAULT
{
    const std::string default_name = "High-Quality Hardened Concrete";
    const double default_density = 2650.0; // kg/m^3
    const double default_compressive_strength = 60.0e6; // 60 MPa
};

struct RODS_FROM_GOD_DEFAULT
{
    const std::string default_name = "Orbital Tungsten Kinetic Penetrator (Rods from God)";
    const double default_length = 6.1;
    const double default_diameter = 0.3;
    const double default_total_mass = 8300.0;
    const double default_explosive_mass = 0.0;     // 0 kg explosive (pure kinetic energy weapon)
    const double default_casing_density = 19300.0; // High-density Tungsten
    const double default_yield_strength =
        0.0; // 0 GPa (hydrodynamic erosion dominated at hypervelocity)
};

#endif // DEFAULT_HPP
