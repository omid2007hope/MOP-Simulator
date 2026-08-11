// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <numbers>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"

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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
