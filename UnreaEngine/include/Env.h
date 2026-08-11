// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <numbers>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"



UCLASS()
class AEnviornment : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnviornment();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enviornment Configuration")
	FEnviornment EnviornmentPhysicsData;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
