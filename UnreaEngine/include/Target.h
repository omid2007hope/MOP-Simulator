// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <numbers>
#include <string>
#include <vector>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"


// Target layer specification
struct TargetLayer {
	std::string material_name;
	double thickness = 0.0;
	double density = 0.0;
	double compressive_strength = 0.0;
	double rebar_volume_fraction = 0.0;
	double rebar_yield_strength = 0.0;
	double pulverized_depth = 0.0;
	double hugoniot_c0 = 0.0;
	double hugoniot_s = 0.0;
};

// Target material specification
struct Target {
	std::string name;
	std::vector<TargetLayer> layers;
	void pulverizeDepth(double breachDepth) {
		double currentDepthAcc = 0.0;
		for (auto& layer : layers) {
			if (breachDepth > currentDepthAcc) {
				layer.pulverized_depth = std::max(
					layer.pulverized_depth,
					std::min(layer.thickness, breachDepth - currentDepthAcc));
			}
			currentDepthAcc += layer.thickness;
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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
