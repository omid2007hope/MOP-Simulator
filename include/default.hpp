// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef DEFAULT_HPP
#define DEFAULT_HPP

#include <string>
#include <vector>
#include "simulation.hpp"

inline const Projectile MOP_DEFAULT = {.name = "GBU-57 Massive Ordnance Penetrator (MOP)",
                                       .length = 6.2,
                                       .diameter = 0.8,
                                       .curvature_noseReduce = 4.8,
                                       .total_mass = 13600.0,
                                       .explosive_mass = 2400.0,
                                       .casing_density = 7800.0,
                                       .yield_strength = 2.0e9,
                                       .specific_heat = 460.0,
                                       .melting_point = 1800.0,
                                       .heat_of_fusion = 272000.0,
                                       .area_moment_inertia = 0.02};

inline const Target CONCRETE_DEFAULT = {
    .name = "High-Quality Hardened Concrete Structure",
    .layers = {{"Soil Overburden", 2.0, 1500.0, 10.0e6, 0.0, 0.0},
               {"Heavily Reinforced Concrete", 5.0, 2650.0, 60.0e6, 0.02, 400.0e6},
               {"Solid Granite Rock", 50.0, 2750.0, 130.0e6, 0.0, 0.0}}};

inline const Projectile RODS_FROM_GOD_DEFAULT = {
    .name = "Orbital Tungsten Kinetic Penetrator (Rods from God)",
    .length = 6.1,
    .diameter = 0.3,
    .curvature_noseReduce = 1.8,
    .total_mass = 8300.0,
    .explosive_mass = 0.0,
    .casing_density = 19300.0,
    .yield_strength = 0.0,
    .specific_heat = 134.0,
    .melting_point = 3695.0,
    .heat_of_fusion = 192000.0,
    .area_moment_inertia = 0.0004};

#endif // DEFAULT_HPP
