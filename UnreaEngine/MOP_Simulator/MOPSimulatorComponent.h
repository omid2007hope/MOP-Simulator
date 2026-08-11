// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MOPTypes.h"
#include "MOPSimulatorComponent.generated.h"

// Forward declarations
class UMOPProjectileDataAsset;
class UMOPTargetDataAsset;


DECLARE_LOG_CATEGORY_EXTERN(LogMOPSim, Log, All);


// -----------------------------------------------------------------------
// UMOPSimulatorComponent
//
// Attach to any Actor (typically ATarget) to run a full MOP impact
// simulation and visualize the results inside the UE editor viewport.
//
// Workflow:
//   1. BeginPlay → RunSimulation() executes synchronously.
//   2. ImpactSimulator::simulate() computes all TelemetryFrames.
//   3. DrawDebugVisualization() paints the drop arc and penetration
//      tunnel using DrawDebugLine / DrawDebugSphere / DrawDebugString.
//   4. TickComponent() animates the owning Actor through the recorded
//      frames in real-time (drop then penetration).
// -----------------------------------------------------------------------
UCLASS(ClassGroup = (MOP), meta = (BlueprintSpawnableComponent),
       DisplayName = "MOP Simulator Component")
class MOP_SIMULATOR_API UMOPSimulatorComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UMOPSimulatorComponent();

	// ---- Configuration ------------------------------------------------

	// Drop scenario (altitude, velocity, flight path angle, obliquity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Configuration")
	FMOPImpactScenario Scenario;

	// Projectile physics data — overridden by ProjectileAsset if set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Configuration")
	FMOPProjectile Projectile;

	// Target layer stack — overridden by TargetAsset if set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Configuration")
	FMOPTarget Target;

	// Optional DataAsset override for the projectile (Editor-assigned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Data Assets")
	TObjectPtr<UMOPProjectileDataAsset> ProjectileAsset;

	// Optional DataAsset override for the target (Editor-assigned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Data Assets")
	TObjectPtr<UMOPTargetDataAsset> TargetAsset;

	// ---- Visualization ------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Visualization")
	bool bDrawDropTrajectory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Visualization")
	bool bDrawPenetrationTunnel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Visualization")
	bool bDrawLayerBoundaries = true;

	// How long (seconds) the debug draw lines/spheres persist in the viewport
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Visualization",
	          meta = (ClampMin = "1.0", ClampMax = "300.0"))
	float DebugDrawDuration = 60.0f;

	// Scale for all debug spheres (1.0 = 50 cm radius at impact point)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Visualization",
	          meta = (ClampMin = "0.1"))
	float DebugScale = 1.0f;

	// ---- Results (read-only, exposed to Blueprint) --------------------

	UPROPERTY(BlueprintReadOnly, Category = "MOP|Result")
	FMOPSimulationResult LastResult;

	UPROPERTY(BlueprintReadOnly, Category = "MOP|Result")
	bool bSimulationComplete = false;

	// ---- Blueprint-callable API ---------------------------------------

	// Runs the full simulation immediately (synchronous). Safe to call
	// multiple times (e.g. from a Blueprint trigger on key press).
	UFUNCTION(BlueprintCallable, Category = "MOP|Simulation",
	          meta = (DisplayName = "Run MOP Simulation"))
	void RunSimulation();

	// Clears all persistent debug draw from the last simulation run.
	UFUNCTION(BlueprintCallable, Category = "MOP|Simulation")
	void ClearVisualization();

	// Resets playback to the beginning without re-running the simulation.
	UFUNCTION(BlueprintCallable, Category = "MOP|Simulation")
	void ResetPlayback();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Draws the full trajectory + penetration + layer boundaries
	void DrawDebugVisualization(const FMOPSimulationResult& Result);

	// Writes key telemetry events to UE Output Log (LogMOPSim category)
	void LogTelemetry(const FMOPSimulationResult& Result);

	// Reads DataAssets if assigned and merges into Projectile / Target
	void ApplyDataAssetOverrides();

	// ---- Tick-based playback state -----------------------------------
	bool   bPlaybackActive    = false;
	float  PlaybackTime       = 0.0f;
	int32  PlaybackFrameIdx   = 0;
	bool   bInPenetrationPhase = false;
	FVector ImpactWorldLocation = FVector::ZeroVector;

	// Accumulated horizontal offset during drop (for trajectory animation)
	double PlaybackAccumX = 0.0;
	double PlaybackPrevFrameTime = 0.0;
};
