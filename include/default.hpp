// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef DEFAULT_HPP
#define DEFAULT_HPP

#include <string>
#include <vector>
#include "simulation.hpp"

inline const Projectile MOP_DEFAULT = {
    .name = "GBU-57 Massive Ordnance Penetrator (MOP)",
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
    .area_moment_inertia = 0.02,
    .elastic_modulus = 200.0e9,
    .casing_wall_thickness = 0.1,
    .hugoniot_c0 = 4570.0,
    .hugoniot_s = 1.49,
    .explosive_critical_energy = 15.0e12};

inline const Target CONCRETE_DEFAULT = {
    .name = "High-Quality Hardened Concrete Structure",
    .layers = {{.material_name = "Soil Overburden",
                .thickness = 2.0,
                .density = 1500.0,
                .compressive_strength = 10.0e6,
                .rebar_volume_fraction = 0.0,
                .rebar_yield_strength = 0.0,
                .hugoniot_c0 = 1500.0,
                .hugoniot_s = 1.5},
               {.material_name = "Heavily Reinforced Concrete",
                .thickness = 5.0,
                .density = 2650.0,
                .compressive_strength = 60.0e6,
                .rebar_volume_fraction = 0.02,
                .rebar_yield_strength = 400.0e6,
                .hugoniot_c0 = 3200.0,
                .hugoniot_s = 1.9},
               {.material_name = "Solid Granite Rock",
                .thickness = 50.0,
                .density = 2750.0,
                .compressive_strength = 130.0e6,
                .rebar_volume_fraction = 0.0,
                .rebar_yield_strength = 0.0,
                .hugoniot_c0 = 3680.0,
                .hugoniot_s = 1.35}}};

inline const Projectile RODS_FROM_GOD_DEFAULT = {
    .name = "Orbital Tungsten Kinetic Penetrator (Rods from God)",
    .length = 6.1,
    .diameter = 0.3,
    .curvature_noseReduce = 1.8,
    .total_mass = 8300.0,
    .explosive_mass = 0.0,
    .casing_density = 19300.0,
    .yield_strength = 1.51e9, // Tungsten heavy alloy dynamic flow stress (kinetic rods still have real casing strength)
    .specific_heat = 134.0,
    .melting_point = 3695.0,
    .heat_of_fusion = 192000.0,
    .area_moment_inertia = 0.0004,
    .elastic_modulus = 400.0e9,
    .casing_wall_thickness = 0.15,
    .hugoniot_c0 = 4030.0,
    .hugoniot_s = 1.24,
    .explosive_critical_energy = 15.0e12};

#endif // DEFAULT_HPP
