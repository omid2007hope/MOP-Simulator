// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


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

UCLASS()
class AProjectile : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Configuration")
	FProjectile ProjectilePhysicsData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> ProjectileMesh;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Bomb's data
	FProjectile GetPhysicsData() const {
		return ProjectilePhysicsData;
	}
};
