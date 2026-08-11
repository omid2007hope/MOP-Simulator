// Copyright Epic Games, Inc. All Rights Reserved.

#include "MOP_Simulator.h"

#include "Modules/ModuleManager.h"


IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, MOP_Simulator, "MOP_Simulator");

// Fill out your copyright notice in the Description page of Project Settings.

// Sets default values
AProjectile::AProjectile() {
	PrimaryActorTick.bCanEverTick = true;
}

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

	// Trigger after impact
	TargetMesh->OnComponentHit.AddDynamic(this, &ATarget::OnTargetHit);
}

// Impact
void ATarget::ReceiveImpact() {};
// on Impact Event
void ATarget::OnTargetHit(UPrimitiveComponent* HitComponent,
			  AActor* OtherActor,
			  UPrimitiveComponent* OtherComp,
			  FVector NormalImpulse,
			  const FHitResult& Hit) {
	// 1. Check if the thing that hit us is actually the Projectile!
	AProjectile* HittingBomb = Cast<AProjectile>(OtherActor);

	if (HittingBomb != nullptr) {

		// Receive bomb's data
		FProjectile BombData = HittingBomb->GetPhysicsData();

		// IT IS THE BOMB!
		// 2. THIS is where you paste your MASSIVE while-loop from `simulation.cpp`!
		// You will calculate the penetration depth using `TargetPhysicsData.Layers`
		// and the bomb's mass!
	}
}


// Called when the game starts or when spawned
void ATarget::BeginPlay() {
	Super::BeginPlay();
}

// Called every frame
void ATarget::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}