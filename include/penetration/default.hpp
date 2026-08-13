// Copyright (c) 2026 Omid Teimory. All Rights Reserved

#ifndef DEFAULT_HPP
#define DEFAULT_HPP

// packages
#include <string>
#include <vector>

// files
#include "simulation.hpp"




inline const ImpactScenario Midnight_Hammer_Scenario = {
	.name = "Midnight Hammer",
	.altitude_ft = 50000.0,
	.velocity = 250.0,
	.flight_path_angle = 357.5,
	.obliquity_angle = 0.0,
	.angle_of_attack = 0.0,
};


// Atmospheric state at a given geometric altitude (US Standard Atmosphere 1976)

inline const AtmosphereState Midnight_Hammer_Atmosphere = {
	// 15 degree Celsius
	.temperature_K = 288.15,
	.pressure_Pa = 101325.0,
	.density_kgm3 = 1.225,
	.speed_of_sound_ms = 340.3,
};




inline const Aircraft B2_Sprit_Strategic_Bomber = {
	.bomber_totalMass = 152634.0,
	.bomber_wingArea = 478.0,
	.bomber_liftCurveSlope = 5.74,
};




inline const Projectile Midnight_Hammer_projectile = {
	.name = "GBU-57 Massive Ordnance Penetrator (MOP)",
	.length = 6.2,
	.diameter = 0.8,
	.curvature_noseReduce = 4.8,
	.total_mass = 13600.0,
	.explosive_mass = 2423.0,
	.casing_density = 7850.0,
	.yield_strength = 2.5e9,
	.area_moment_inertia = 0.0201,
	.elastic_modulus = 200.0e9,
	.casing_wall_thickness = 0.1,
	.hugoniot_c0 = 4570.0,
	.hugoniot_s = 1.49,
	.explosive_critical_energy = 3.0e15,
	.explosive_energy_j_per_kg = 5.2e6,
	// specific_heat determines how much the temperature changes for a given amount of energy.
	.specific_heat = 460.0,
	// The fixed temperature threshold where solid and liquid phases coexist in equilibrium. For water, this is 0°C, but it varies by material.
	.melting_point = 1800.0,
	// the amount of thermal energy required to change a unit mass of a substance from solid to liquid at its melting point without changing its temperature. This energy breaks the intermolecular bonds holding the solid structure together.
	.heat_of_fusion = 272000.0,
};




inline const Target Midnight_Hammer_Target = {
	.name = "Fordow-like Hardened Mountain Facility",
	.layers = {{.material_name = "Soil/Overburden + Surface Cap",
		    .thickness = 3.0,
		    .density = 1800.0,
		    .compressive_strength = 20.0e6,
		    .rebar_volume_fraction = 0.0,
		    .rebar_yield_strength = 0.0,
		    .hugoniot_c0 = 1800.0,
		    .hugoniot_s = 1.5},
		   {.material_name = "Reinforced Concrete (facility structure / shaft lining)",
		    .thickness = 5.0,
		    .density = 2650.0,
		    .compressive_strength = 70.0e6,
		    .rebar_volume_fraction = 0.04,
		    .rebar_yield_strength = 500.0e6,
		    .hugoniot_c0 = 3200.0,
		    .hugoniot_s = 1.9},
		   {.material_name = "Solid Mountain Rock (limestone/granite-like)",
		    .thickness = 90.0,
		    .density = 2750.0,
		    .compressive_strength = 170.0e6,
		    .rebar_volume_fraction = 0.0,
		    .rebar_yield_strength = 0.0,
		    .hugoniot_c0 = 3800.0,
		    .hugoniot_s = 1.35}}};




inline const Projectile MOP_DEFAULT = {.name = "GBU-57 Massive Ordnance Penetrator (MOP)",
				       .length = 6.2,
				       .diameter = 0.8,
				       .curvature_noseReduce = 4.8,
				       .total_mass = 13600.0,
				       .explosive_mass = 2400.0,
				       .casing_density = 7800.0,
				       .yield_strength = 2.0e9,
				       .area_moment_inertia = 0.02,
				       .elastic_modulus = 200.0e9,
				       .casing_wall_thickness = 0.1,
				       .hugoniot_c0 = 4570.0,
				       .hugoniot_s = 1.49,
				       .explosive_critical_energy = 6.0e16, // Walker-Wasley P^2*tau threshold for AFX-757 insensitive munition
				       .explosive_energy_j_per_kg = 5.2e6,
				       .specific_heat = 460.0,
				       .melting_point = 1800.0,
				       .heat_of_fusion = 272000.0};




inline const Target CONCRETE_DEFAULT = {.name = "High-Quality Hardened Concrete Structure",
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
	.yield_strength = 1.51e9,
	.area_moment_inertia = 0.0004,
	.elastic_modulus = 400.0e9,
	.casing_wall_thickness = 0.15,
	.hugoniot_c0 = 4030.0,
	.hugoniot_s = 1.24,
	.explosive_critical_energy = 3.0e15,
	.explosive_energy_j_per_kg = 0.0,
	.specific_heat = 134.0,
	.melting_point = 3695.0,
	.heat_of_fusion = 192000.0};

#endif // DEFAULT_HPP
