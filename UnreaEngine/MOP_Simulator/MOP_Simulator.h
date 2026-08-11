// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <numbers>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"


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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Configuration")
	FProjectile ProjectilePhysicsData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> ProjectileMesh;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTargetHit(UPrimitiveComponent* HitComponent,
			 AActor* OtherActor,
			 UPrimitiveComponent* OtherComp,
			 FVector NormalImpulse,
			 const FHitResult& Hit);

	// Bomb's data
	FProjectile GetPhysicsData() const {
		return ProjectilePhysicsData;
	}
};
