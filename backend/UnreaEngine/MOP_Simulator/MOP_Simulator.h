// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MOPTypes.h"
#include "MOP_Simulator.generated.h"

// Forward declaration — full include in .cpp to avoid circular dependency
class UMOPSimulatorComponent;

USTRUCT(BlueprintType)
struct FProjectile {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	FString Name = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double Length = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double Diameter = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double CurvatureNoseReduce = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double TotalMass = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double ExplosiveMass = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double CasingDensity = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double YieldStrength = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double AreaMomentInertia = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double ElasticModulus = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double CasingWallThickness = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double HugoniotC0 = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double HugoniotS = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double ExplosiveCriticalEnergy = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double ExplosiveEnergyJPerKg = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double SpecificHeat = 460.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double MeltingPoint = 1800.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Properties")
	double HeatOfFusion = 272000.0;
};

// Target layer specification
USTRUCT(BlueprintType)
struct FTargetLayer {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	FString MaterialName = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double Thickness = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double Density = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double CompressiveStrength = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double RebarVolumeFraction = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double RebarYieldStrength = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double PulverizedDepth = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double HugoniotC0 = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	double HugoniotS = 0.0;
};

// Target material specification
USTRUCT(BlueprintType)
struct FTarget {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	FString Name = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Properties")
	TArray<FTargetLayer> Layers;

	void PulverizeDepth(double BreachDepth) {
		double CurrentDepthAcc = 0.0;
		for (FTargetLayer& Layer : Layers) {
			if (BreachDepth > CurrentDepthAcc) {
				Layer.PulverizedDepth = FMath::Max(
					Layer.PulverizedDepth,
					FMath::Min(Layer.Thickness, BreachDepth - CurrentDepthAcc));
			}
			CurrentDepthAcc += Layer.Thickness;
		}
	}
};

USTRUCT(BlueprintType)
struct FImpactScenario {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Scenario")
	FString Name = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Scenario")
	double AltitudeFt = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Scenario")
	double Velocity = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Scenario")
	double FlightPathAngle = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Scenario")
	double ObliquityAngle = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Scenario")
	double AngleOfAttack = 0.0;
};

inline const FImpactScenario Midnight_Hammer_Scenario = {
	.Name = TEXT("Midnight Hammer"),
	.AltitudeFt = 50000.0,
	.Velocity = 250.0,
	.FlightPathAngle = 357.5,
	.ObliquityAngle = 0.0,
	.AngleOfAttack = 0.0,
};

USTRUCT(BlueprintType)
struct FAtmosphereState {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere State")
	double TemperatureK = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere State")
	double PressurePa = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere State")
	double DensityKgm3 = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere State")
	double SpeedOfSoundMs = 0.0;
};

inline const FAtmosphereState Midnight_Hammer_Atmosphere = {
	.TemperatureK = 288.15, // 15 degree Celsius
	.PressurePa = 101325.0,
	.DensityKgm3 = 1.225,
	.SpeedOfSoundMs = 340.3
};

inline const FProjectile MOP_DEFAULT = {
	.Name = TEXT("GBU-57 Massive Ordnance Penetrator (MOP)"),
	.Length = 6.2,
	.Diameter = 0.8,
	.CurvatureNoseReduce = 4.8,
	.TotalMass = 13600.0,
	.ExplosiveMass = 2400.0,
	.CasingDensity = 7800.0,
	.YieldStrength = 2.0e9,
	.AreaMomentInertia = 0.02,
	.ElasticModulus = 200.0e9,
	.CasingWallThickness = 0.1,
	.HugoniotC0 = 4570.0,
	.HugoniotS = 1.49,
	.ExplosiveCriticalEnergy = 6.0e16, // Walker-Wasley P^2*tau threshold for AFX-757 insensitive munition
	.ExplosiveEnergyJPerKg = 5.2e6,
	.SpecificHeat = 460.0,
	.MeltingPoint = 1800.0,
	.HeatOfFusion = 272000.0
};

inline const FTarget CONCRETE_DEFAULT = {
	.Name = TEXT("High-Quality Hardened Concrete Structure"),
	.Layers = {
		{
			.MaterialName = TEXT("Soil Overburden"),
			.Thickness = 2.0,
			.Density = 1500.0,
			.CompressiveStrength = 10.0e6,
			.RebarVolumeFraction = 0.0,
			.RebarYieldStrength = 0.0,
			.PulverizedDepth = 0.0,
			.HugoniotC0 = 1500.0,
			.HugoniotS = 1.5
		},
		{
			.MaterialName = TEXT("Heavily Reinforced Concrete"),
			.Thickness = 5.0,
			.Density = 2650.0,
			.CompressiveStrength = 60.0e6,
			.RebarVolumeFraction = 0.02,
			.RebarYieldStrength = 400.0e6,
			.PulverizedDepth = 0.0,
			.HugoniotC0 = 3200.0,
			.HugoniotS = 1.9
		},
		{
			.MaterialName = TEXT("Solid Granite Rock"),
			.Thickness = 50.0,
			.Density = 2750.0,
			.CompressiveStrength = 130.0e6,
			.RebarVolumeFraction = 0.0,
			.RebarYieldStrength = 0.0,
			.PulverizedDepth = 0.0,
			.HugoniotC0 = 3680.0,
			.HugoniotS = 1.35
		}
	}
};

inline const FProjectile Midnight_Hammer_projectile = {
	.Name = TEXT("GBU-57 Massive Ordnance Penetrator (MOP)"),
	.Length = 6.2,
	.Diameter = 0.8,
	.CurvatureNoseReduce = 4.8,
	.TotalMass = 13600.0,
	.ExplosiveMass = 2423.0,
	.CasingDensity = 7850.0,
	.YieldStrength = 2.5e9,
	.AreaMomentInertia = 0.0201,
	.ElasticModulus = 200.0e9,
	.CasingWallThickness = 0.1,
	.HugoniotC0 = 4570.0,
	.HugoniotS = 1.49,
	.ExplosiveCriticalEnergy = 3.0e15,
	.ExplosiveEnergyJPerKg = 5.2e6,
	.SpecificHeat = 460.0,
	.MeltingPoint = 1800.0,
	.HeatOfFusion = 272000.0
};

UCLASS()
class AProjectile : public AActor {
	GENERATED_BODY()

public:
	AProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Configuration")
	FProjectile ProjectilePhysicsData;

	UFUNCTION(BlueprintCallable, Category = "Physics")
	FProjectile GetPhysicsData() const {
		return ProjectilePhysicsData;
	}
};

UCLASS()
class ATarget : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Configuration")
	FTarget TargetPhysicsData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> TargetMesh;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Core physics simulation component.
	// Runs ImpactSimulator::simulate() on BeginPlay, draws debug trajectory
	// in viewport, and animates the attached projectile via TickComponent.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMOPSimulatorComponent> SimulatorComponent;

	UFUNCTION()
	void ReceiveImpact();

	UFUNCTION()
	void OnTargetHit(UPrimitiveComponent* HitComponent,
			 AActor* OtherActor,
			 UPrimitiveComponent* OtherComp,
			 FVector NormalImpulse,
			 const FHitResult& Hit);
};
