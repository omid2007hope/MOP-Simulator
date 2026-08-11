// Fill out your copyright notice in the Description page of Project Settings.


#include "Target.h"

// Sets default values
ATarget::ATarget() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// assign a mesh
	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	RootComponent = TargetMesh; // Make the mesh the root of the entire Actor!

	// Set Default Target Data (Fordow-like Hardened Mountain Facility)
	TargetPhysicsData.Name = TEXT("Fordow-like Hardened Mountain Facility");

	// Layer 1: Soil/Overburden
	FTargetLayer SoilLayer;
	SoilLayer.MaterialName = TEXT("Soil/Overburden + Surface Cap");
	SoilLayer.Thickness = 3.0;
	SoilLayer.Density = 1800.0;
	SoilLayer.CompressiveStrength = 20.0e6;
	SoilLayer.HugoniotC0 = 1800.0;
	SoilLayer.HugoniotS = 1.5;
	TargetPhysicsData.Layers.Add(SoilLayer);

	// Layer 2: Reinforced Concrete
	FTargetLayer ConcreteLayer;
	ConcreteLayer.MaterialName =
		TEXT("Reinforced Concrete (facility structure / shaft lining)");
	ConcreteLayer.Thickness = 5.0;
	ConcreteLayer.Density = 2650.0;
	ConcreteLayer.CompressiveStrength = 70.0e6;
	ConcreteLayer.RebarVolumeFraction = 0.04;
	ConcreteLayer.RebarYieldStrength = 500.0e6;
	ConcreteLayer.HugoniotC0 = 3200.0;
	ConcreteLayer.HugoniotS = 1.9;
	TargetPhysicsData.Layers.Add(ConcreteLayer);

	// Layer 3: Solid Mountain Rock
	FTargetLayer RockLayer;
	RockLayer.MaterialName = TEXT("Solid Mountain Rock (limestone/granite-like)");
	RockLayer.Thickness = 90.0;
	RockLayer.Density = 2750.0;
	RockLayer.CompressiveStrength = 170.0e6;
	RockLayer.HugoniotC0 = 3800.0;
	RockLayer.HugoniotS = 1.35;
	TargetPhysicsData.Layers.Add(RockLayer);
}

// Impact
void ATarget::ReceiveImpact() {};

// Called when the game starts or when spawned
void ATarget::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void ATarget::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}
