// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MOPTypes.generated.h"


// -----------------------------------------------------------------------
// Physics constants (read-only, set once at startup)
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPPhysicsConstants {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double Gravity = 9.81;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double PI = 3.14159265358979323846;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double FrictionFactor = 0.1;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double UniversalGasConstant = 8.31432;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double MolarMassAir = 0.0289644;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double AdiabaticIndexAir = 1.4;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double EarthRadius = 6356766.0;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Physics")
	double SpeedOfSound = 343.0;
};


// -----------------------------------------------------------------------
// Impact scenario (drop conditions)
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPImpactScenario {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Simulation")
	FString Name = TEXT("Midnight Hammer");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Simulation")
	double AltitudeFt = 50000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Simulation")
	double Velocity = 250.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Simulation")
	double FlightPathAngle = 357.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Simulation")
	double ObliquityAngle = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Simulation")
	double AngleOfAttack = 0.0;
};


// -----------------------------------------------------------------------
// US Standard Atmosphere 1976 state at a given altitude
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPAtmosphereState {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MOP Atmosphere")
	double TemperatureK = 288.15;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Atmosphere")
	double PressurePa = 101325.0;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Atmosphere")
	double DensityKgm3 = 1.225;

	UPROPERTY(BlueprintReadOnly, Category = "MOP Atmosphere")
	double SpeedOfSoundMs = 340.3;
};


// -----------------------------------------------------------------------
// Projectile physical specification (e.g. GBU-57 MOP)
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPProjectile {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	FString Name = TEXT("GBU-57 Massive Ordnance Penetrator (MOP)");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double Length = 6.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double Diameter = 0.8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double CurvatureNoseReduce = 4.8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double TotalMass = 13600.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double ExplosiveMass = 2423.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double CasingDensity = 7850.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double YieldStrength = 2.5e9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double AreaMomentInertia = 0.0201;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double ElasticModulus = 200.0e9;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double CasingWallThickness = 0.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double HugoniotC0 = 4570.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double HugoniotS = 1.49;

	// Walker-Wasley P^2*tau threshold for AFX-757 insensitive munition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double ExplosiveCriticalEnergy = 3.0e15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double ExplosiveEnergyJPerKg = 5.2e6;

	// How much temperature changes per unit of energy deposited
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double SpecificHeat = 460.0;

	// Temperature (K) at which solid and liquid phases coexist
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double MeltingPoint = 1800.0;

	// Energy required to melt unit mass without temperature change
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Projectile")
	double HeatOfFusion = 272000.0;
};


// -----------------------------------------------------------------------
// Single target strata layer
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPTargetLayer {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	FString MaterialName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	double Thickness = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	double Density = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	double CompressiveStrength = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	double RebarVolumeFraction = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	double RebarYieldStrength = 0.0;

	// Cumulative pulverized depth from previous strikes (updated by simulation)
	UPROPERTY(BlueprintReadOnly, Category = "MOP Target")
	double PulverizedDepth = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	double HugoniotC0 = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	double HugoniotS = 0.0;
};


// -----------------------------------------------------------------------
// Multi-layer target structure
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPTarget {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Target")
	TArray<FMOPTargetLayer> Layers;
};


// -----------------------------------------------------------------------
// One recorded telemetry frame (drop or penetration)
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPTelemetryFrame {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Time = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Altitude = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Depth = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Velocity = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Mach = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double DynamicPressure = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") bool bIsSonicBoom = false;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Heat = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double GForce = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double PitchRad = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") bool bIsEroding = false;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double DIF = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double RemainingLength = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double ObliquityDeg = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double CurrentVX = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double CurrentVY = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double DragCoefficient = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double DragForce = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double GuidancePull = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Up = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double Us = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double PShock = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double TransmittedPressure = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double ShockEnergy = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double AsymmetricForce = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double BendingMoment = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double MaxBendingStress = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double StrainRate = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double EffectiveStrength = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double TunnelForce = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double InterfaceErosionVelocity = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double HeatRate = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double ExcessHeat = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double MassLoss = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Telemetry") double EffectiveLinearDensity = 0.0;
};


// -----------------------------------------------------------------------
// Full simulation result for one scenario run
// -----------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FMOPSimulationResult {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") FString ScenarioName;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double AltitudeFt = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double Velocity = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double MachNumber = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double KineticEnergy = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double DynamicPressure = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") bool bCasingFailure = false;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") bool bPrematureDetonation = false;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double HydroPenetration = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double RigidPenetration = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ActualPenetrationDepth = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ShockDamageProbPercent = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") bool bExplosiveChargeSurvives = true;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") bool bIsKineticRod = false;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") FString Regime;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") FString OutcomeSummary;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") bool bErosionOccurred = false;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double FinalRodLength = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ErosionLengthLost = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double DynamicIncreaseFactor = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double BarWaveSpeed = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ShockPressureGpaPeak = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ShockPulseDurationUs = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double PreviousStrikeDepth = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double CumulativeBreachDepth = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ExplosiveMass = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ExplosionScale = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double CraterWideRadius = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double CraterNarrowRadius = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double CameraShakeMagnitude = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double TimeScalePen = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ImpactVelocity = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double ImpactPitch = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double KineticShockJoules = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") double TotalExplosiveYield = 0.0;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") TArray<double> LayerBottomDepths;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") TArray<FMOPTelemetryFrame> DropFrames;
	UPROPERTY(BlueprintReadOnly, Category = "MOP Result") TArray<FMOPTelemetryFrame> PenetrationFrames;
};
