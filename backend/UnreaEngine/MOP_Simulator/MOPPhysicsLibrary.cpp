// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

#include "MOPPhysicsLibrary.h"
#include "MOPSimulationBridge.h"

// Raw C++ physics function declarations (definitions compiled in MOPSimulationCore.cpp)
#include "environment_physics.hpp"


// -----------------------------------------------------------------------
// FMOPAtmosphereState GetStandardAtmosphere
// -----------------------------------------------------------------------
FMOPAtmosphereState UMOPPhysicsLibrary::GetStandardAtmosphere(double AltitudeM) {
	PhysicsConstants Cons;
	AtmosphereState Raw = EnvironmentPhysics::standardAtmosphere(AltitudeM, Cons);

	FMOPAtmosphereState Out;
	Out.TemperatureK    = Raw.temperature_K;
	Out.PressurePa      = Raw.pressure_Pa;
	Out.DensityKgm3     = Raw.density_kgm3;
	Out.SpeedOfSoundMs  = Raw.speed_of_sound_ms;
	return Out;
}


// -----------------------------------------------------------------------
// double GetMachDependentDrag
// -----------------------------------------------------------------------
double UMOPPhysicsLibrary::GetMachDependentDrag(double Mach,
                                                 double BaseCd,
                                                 const FMOPProjectile& Projectile) {
	PhysicsConstants Cons;
	return EnvironmentPhysics::getMachDependentDrag(Mach, BaseCd,
	                                                MOPBridge::ToRaw(Projectile), Cons);
}


// -----------------------------------------------------------------------
// double ComputeDIF
// -----------------------------------------------------------------------
double UMOPPhysicsLibrary::ComputeDIF(double StrainRatePerS, double FcStaticPa) {
	return EnvironmentPhysics::computeDIF(StrainRatePerS, FcStaticPa);
}


// -----------------------------------------------------------------------
// double SolveInterfaceVelocity
// -----------------------------------------------------------------------
double UMOPPhysicsLibrary::SolveInterfaceVelocity(double V, double RhoP, double RhoT,
                                                    double Yp, double Rt) {
	return EnvironmentPhysics::solveInterfaceVelocity(V, RhoP, RhoT, Yp, Rt);
}


// -----------------------------------------------------------------------
// double FlightControlTrim
// -----------------------------------------------------------------------
double UMOPPhysicsLibrary::FlightControlTrim(double FlightPathAngle,
                                               double BomberVelocity,
                                               double BombMass,
                                               double CurveSlope,
                                               double WingArea,
                                               double AirDensity) {
	PhysicsConstants Cons;
	return EnvironmentPhysics::flightControlTrim(FlightPathAngle, BomberVelocity,
	                                             BombMass, CurveSlope,
	                                             WingArea, AirDensity, Cons);
}
