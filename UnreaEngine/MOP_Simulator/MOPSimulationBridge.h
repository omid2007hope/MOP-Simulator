// Copyright (c) 2026 Omid Teimory. All Rights Reserved.
//
// Header-only bridge: converts between UE FMOPXxx structs and the raw C++
// structs used by the standalone ImpactSimulator engine (simulation.hpp).
// No .cpp needed — all inline.

#pragma once

#include "CoreMinimal.h"
#include "MOPTypes.h"

// Raw C++ physics engine structs (standalone, no UE deps)
#include "simulation.hpp"


namespace MOPBridge {

// -----------------------------------------------------------------------
// UE → Raw C++ (input to ImpactSimulator)
// -----------------------------------------------------------------------

inline Projectile ToRaw(const FMOPProjectile& In) {
	Projectile Out;
	Out.name                   = TCHAR_TO_UTF8(*In.Name);
	Out.length                 = In.Length;
	Out.diameter               = In.Diameter;
	Out.curvature_noseReduce   = In.CurvatureNoseReduce;
	Out.total_mass             = In.TotalMass;
	Out.explosive_mass         = In.ExplosiveMass;
	Out.casing_density         = In.CasingDensity;
	Out.yield_strength         = In.YieldStrength;
	Out.area_moment_inertia    = In.AreaMomentInertia;
	Out.elastic_modulus        = In.ElasticModulus;
	Out.casing_wall_thickness  = In.CasingWallThickness;
	Out.hugoniot_c0            = In.HugoniotC0;
	Out.hugoniot_s             = In.HugoniotS;
	Out.explosive_critical_energy  = In.ExplosiveCriticalEnergy;
	Out.explosive_energy_j_per_kg  = In.ExplosiveEnergyJPerKg;
	Out.specific_heat          = In.SpecificHeat;
	Out.melting_point          = In.MeltingPoint;
	Out.heat_of_fusion         = In.HeatOfFusion;
	return Out;
}

inline Target ToRaw(const FMOPTarget& In) {
	Target Out;
	Out.name = TCHAR_TO_UTF8(*In.Name);
	for (const FMOPTargetLayer& L : In.Layers) {
		TargetLayer Raw;
		Raw.material_name         = TCHAR_TO_UTF8(*L.MaterialName);
		Raw.thickness             = L.Thickness;
		Raw.density               = L.Density;
		Raw.compressive_strength  = L.CompressiveStrength;
		Raw.rebar_volume_fraction = L.RebarVolumeFraction;
		Raw.rebar_yield_strength  = L.RebarYieldStrength;
		Raw.pulverized_depth      = L.PulverizedDepth;
		Raw.hugoniot_c0           = L.HugoniotC0;
		Raw.hugoniot_s            = L.HugoniotS;
		Out.layers.push_back(Raw);
	}
	return Out;
}

inline ImpactScenario ToRaw(const FMOPImpactScenario& In) {
	ImpactScenario Out;
	Out.name              = TCHAR_TO_UTF8(*In.Name);
	Out.altitude_ft       = In.AltitudeFt;
	Out.velocity          = In.Velocity;
	Out.flight_path_angle = In.FlightPathAngle;
	Out.obliquity_angle   = In.ObliquityAngle;
	Out.angle_of_attack   = In.AngleOfAttack;
	return Out;
}


// -----------------------------------------------------------------------
// Raw C++ → UE (output from ImpactSimulator)
// -----------------------------------------------------------------------

inline FMOPTelemetryFrame FromRaw(const TelemetryFrame& In) {
	FMOPTelemetryFrame Out;
	Out.Time                    = In.time;
	Out.Altitude                = In.altitude;
	Out.Depth                   = In.depth;
	Out.Velocity                = In.velocity;
	Out.Mach                    = In.mach;
	Out.DynamicPressure         = In.dynamic_pressure;
	Out.bIsSonicBoom            = In.is_sonic_boom;
	Out.Heat                    = In.heat;
	Out.GForce                  = In.g_force;
	Out.PitchRad                = In.pitch_rad;
	Out.bIsEroding              = In.is_eroding;
	Out.DIF                     = In.dif;
	Out.RemainingLength         = In.remaining_length;
	Out.ObliquityDeg            = In.obliquity_deg;
	Out.CurrentVX               = In.current_vx;
	Out.CurrentVY               = In.current_vy;
	Out.DragCoefficient         = In.drag_coefficient;
	Out.DragForce               = In.drag_force;
	Out.GuidancePull            = In.guidance_pull;
	Out.Up                      = In.Up;
	Out.Us                      = In.Us;
	Out.PShock                  = In.P_shock;
	Out.TransmittedPressure     = In.transmitted_pressure;
	Out.ShockEnergy             = In.shock_energy;
	Out.AsymmetricForce         = In.asymmetric_force;
	Out.BendingMoment           = In.bending_moment;
	Out.MaxBendingStress        = In.max_bending_stress;
	Out.StrainRate              = In.strain_rate;
	Out.EffectiveStrength       = In.effective_strength;
	Out.TunnelForce             = In.tunnel_force;
	Out.InterfaceErosionVelocity = In.interface_erosion_velocity;
	Out.HeatRate                = In.heat_rate;
	Out.ExcessHeat              = In.excess_heat;
	Out.MassLoss                = In.mass_loss;
	Out.EffectiveLinearDensity  = In.effective_linear_density;
	return Out;
}

inline FMOPSimulationResult FromRaw(const SimulationResult& In) {
	FMOPSimulationResult Out;
	Out.ScenarioName           = UTF8_TO_TCHAR(In.scenario_name.c_str());
	Out.AltitudeFt             = In.altitude_ft;
	Out.Velocity               = In.velocity;
	Out.MachNumber             = In.mach_number;
	Out.KineticEnergy          = In.kinetic_energy;
	Out.DynamicPressure        = In.dynamic_pressure;
	Out.bCasingFailure         = In.casing_failure;
	Out.bPrematureDetonation   = In.premature_detonation;
	Out.HydroPenetration       = In.hydro_penetration;
	Out.RigidPenetration       = In.rigid_penetration;
	Out.ActualPenetrationDepth = In.actual_penetration_depth;
	Out.ShockDamageProbPercent = In.shock_damage_prob_percent;
	Out.bExplosiveChargeSurvives = In.explosive_charge_survives;
	Out.bIsKineticRod          = In.is_kinetic_rod;
	Out.Regime                 = UTF8_TO_TCHAR(In.regime.c_str());
	Out.OutcomeSummary         = UTF8_TO_TCHAR(In.outcome_summary.c_str());
	Out.bErosionOccurred       = In.erosion_occurred;
	Out.FinalRodLength         = In.final_rod_length;
	Out.ErosionLengthLost      = In.erosion_length_lost;
	Out.DynamicIncreaseFactor  = In.dynamic_increase_factor;
	Out.BarWaveSpeed           = In.bar_wave_speed;
	Out.ShockPressureGpaPeak   = In.shock_pressure_gpa_peak;
	Out.ShockPulseDurationUs   = In.shock_pulse_duration_us;
	Out.PreviousStrikeDepth    = In.previous_strike_depth;
	Out.CumulativeBreachDepth  = In.cumulative_breach_depth;
	Out.ExplosiveMass          = In.explosive_mass;
	Out.ExplosionScale         = In.explosion_scale;
	Out.CraterWideRadius       = In.crater_wide_radius;
	Out.CraterNarrowRadius     = In.crater_narrow_radius;
	Out.CameraShakeMagnitude   = In.camera_shake_magnitude;
	Out.TimeScalePen           = In.time_scale_pen;
	Out.ImpactVelocity         = In.impact_velocity;
	Out.ImpactPitch            = In.impact_pitch;
	Out.KineticShockJoules     = In.kinetic_shock_joules;
	Out.TotalExplosiveYield    = In.total_explosive_yield;
	for (double D : In.layer_bottom_depths)    Out.LayerBottomDepths.Add(D);
	for (const TelemetryFrame& F : In.drop_frames)        Out.DropFrames.Add(FromRaw(F));
	for (const TelemetryFrame& F : In.penetration_frames) Out.PenetrationFrames.Add(FromRaw(F));
	return Out;
}


// -----------------------------------------------------------------------
// Default configurations (Midnight Hammer / Fordow scenario)
// -----------------------------------------------------------------------

inline FMOPImpactScenario GetDefaultScenario() {
	FMOPImpactScenario S;
	S.Name            = TEXT("Midnight Hammer");
	S.AltitudeFt      = 50000.0;
	S.Velocity        = 250.0;
	S.FlightPathAngle = 357.5;
	S.ObliquityAngle  = 0.0;
	S.AngleOfAttack   = 0.0;
	return S;
}

inline FMOPProjectile GetDefaultProjectile() {
	FMOPProjectile P;
	P.Name                   = TEXT("GBU-57 Massive Ordnance Penetrator (MOP)");
	P.Length                 = 6.2;
	P.Diameter               = 0.8;
	P.CurvatureNoseReduce    = 4.8;
	P.TotalMass              = 13600.0;
	P.ExplosiveMass          = 2423.0;
	P.CasingDensity          = 7850.0;
	P.YieldStrength          = 2.5e9;
	P.AreaMomentInertia      = 0.0201;
	P.ElasticModulus         = 200.0e9;
	P.CasingWallThickness    = 0.1;
	P.HugoniotC0             = 4570.0;
	P.HugoniotS              = 1.49;
	P.ExplosiveCriticalEnergy = 3.0e15;
	P.ExplosiveEnergyJPerKg  = 5.2e6;
	P.SpecificHeat           = 460.0;
	P.MeltingPoint           = 1800.0;
	P.HeatOfFusion           = 272000.0;
	return P;
}

inline FMOPTarget GetDefaultTarget() {
	FMOPTarget T;
	T.Name = TEXT("Fordow-like Hardened Mountain Facility");

	FMOPTargetLayer Soil;
	Soil.MaterialName        = TEXT("Soil/Overburden + Surface Cap");
	Soil.Thickness           = 3.0;
	Soil.Density             = 1800.0;
	Soil.CompressiveStrength = 20.0e6;
	Soil.HugoniotC0          = 1800.0;
	Soil.HugoniotS           = 1.5;
	T.Layers.Add(Soil);

	FMOPTargetLayer Concrete;
	Concrete.MaterialName        = TEXT("Reinforced Concrete (facility structure / shaft lining)");
	Concrete.Thickness           = 5.0;
	Concrete.Density             = 2650.0;
	Concrete.CompressiveStrength = 70.0e6;
	Concrete.RebarVolumeFraction = 0.04;
	Concrete.RebarYieldStrength  = 500.0e6;
	Concrete.HugoniotC0          = 3200.0;
	Concrete.HugoniotS           = 1.9;
	T.Layers.Add(Concrete);

	FMOPTargetLayer Rock;
	Rock.MaterialName        = TEXT("Solid Mountain Rock (limestone/granite-like)");
	Rock.Thickness           = 90.0;
	Rock.Density             = 2750.0;
	Rock.CompressiveStrength = 170.0e6;
	Rock.HugoniotC0          = 3800.0;
	Rock.HugoniotS           = 1.35;
	T.Layers.Add(Rock);

	return T;
}

} // namespace MOPBridge
