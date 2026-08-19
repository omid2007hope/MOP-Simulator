// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MOPTypes.h"
#include "MOPTargetDataAsset.generated.h"


// -----------------------------------------------------------------------
// UDataAsset for target configuration.
// Create instances in the Content Browser and assign them to
// UMOPSimulatorComponent to represent different facility types
// (Fordow, generic hardened bunker, open ground, etc.).
// -----------------------------------------------------------------------
UCLASS(BlueprintType)
class MOP_SIMULATOR_API UMOPTargetDataAsset : public UDataAsset {
	GENERATED_BODY()

public:
	// Full multi-layer target specification — all fields editable in the UE Editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MOP Target Configuration")
	FMOPTarget TargetData;

	UMOPTargetDataAsset() {
		// Pre-fill with Fordow-like hardened mountain facility (Midnight Hammer defaults).
		TargetData.Name = TEXT("Fordow-like Hardened Mountain Facility");

		FMOPTargetLayer Soil;
		Soil.MaterialName        = TEXT("Soil/Overburden + Surface Cap");
		Soil.Thickness           = 3.0;
		Soil.Density             = 1800.0;
		Soil.CompressiveStrength = 20.0e6;
		Soil.HugoniotC0          = 1800.0;
		Soil.HugoniotS           = 1.5;
		TargetData.Layers.Add(Soil);

		FMOPTargetLayer Concrete;
		Concrete.MaterialName        = TEXT("Reinforced Concrete (facility structure / shaft lining)");
		Concrete.Thickness           = 5.0;
		Concrete.Density             = 2650.0;
		Concrete.CompressiveStrength = 70.0e6;
		Concrete.RebarVolumeFraction = 0.04;
		Concrete.RebarYieldStrength  = 500.0e6;
		Concrete.HugoniotC0          = 3200.0;
		Concrete.HugoniotS           = 1.9;
		TargetData.Layers.Add(Concrete);

		FMOPTargetLayer Rock;
		Rock.MaterialName        = TEXT("Solid Mountain Rock (limestone/granite-like)");
		Rock.Thickness           = 90.0;
		Rock.Density             = 2750.0;
		Rock.CompressiveStrength = 170.0e6;
		Rock.HugoniotC0          = 3800.0;
		Rock.HugoniotS           = 1.35;
		TargetData.Layers.Add(Rock);
	}
};
