// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <numbers>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"



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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
