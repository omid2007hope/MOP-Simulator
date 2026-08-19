// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MOPTypes.h"
#include "MOPPhysicsLibrary.generated.h"


// -----------------------------------------------------------------------
// UMOPPhysicsLibrary
//
// Blueprint-callable wrappers around the EnvironmentPhysics namespace
// from the standalone physics engine (environment_physics.hpp/.cpp).
//
// All functions are pure (no state) — safe to call from any context.
// Implementation in MOPPhysicsLibrary.cpp delegates directly to the
// raw C++ functions compiled in MOPSimulationCore.cpp.
// -----------------------------------------------------------------------
UCLASS()
class MOP_SIMULATOR_API UMOPPhysicsLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	// US Standard Atmosphere 1976 at a given geometric altitude (metres).
	// Returns temperature, pressure, density, and speed of sound.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MOP Physics|Atmosphere",
		meta = (DisplayName = "Get Standard Atmosphere", Keywords = "mop atmosphere altitude air"))
	static FMOPAtmosphereState GetStandardAtmosphere(double AltitudeM);

	// G7 + Modified-Newtonian drag coefficient for the given Mach number.
	// BaseCd is the subsonic reference drag; proj.CurvatureNoseReduce drives hypersonic blending.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MOP Physics|Aerodynamics",
		meta = (DisplayName = "Get Mach Dependent Drag", Keywords = "mop drag mach cd"))
	static double GetMachDependentDrag(double Mach, double BaseCd,
	                                   const FMOPProjectile& Projectile);

	// CEB-FIP Model Code 1990/2010 Dynamic Increase Factor for compressive strength.
	// Maps strain rate and static fc to a dynamic amplification factor.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MOP Physics|Material",
		meta = (DisplayName = "Compute DIF", Keywords = "mop dif dynamic increase factor concrete"))
	static double ComputeDIF(double StrainRatePerS, double FcStaticPa);

	// Tate-Bernoulli quasi-steady interface erosion velocity.
	// Solves 0.5*rho_p*(v-u)^2 + Yp = 0.5*rho_t*u^2 + Rt analytically.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MOP Physics|Penetration",
		meta = (DisplayName = "Solve Interface Velocity", Keywords = "mop erosion interface velocity"))
	static double SolveInterfaceVelocity(double V, double RhoP, double RhoT,
	                                     double Yp, double Rt);

	// Trim angle (degrees) the B-2 Spirit must adjust after releasing the bomb.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MOP Physics|Flight",
		meta = (DisplayName = "Flight Control Trim", Keywords = "mop trim bomber flight angle"))
	static double FlightControlTrim(double FlightPathAngle, double BomberVelocity,
	                                double BombMass, double CurveSlope,
	                                double WingArea, double AirDensity);
};
