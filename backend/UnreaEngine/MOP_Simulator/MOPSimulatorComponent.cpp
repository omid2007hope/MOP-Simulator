// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

#include "MOPSimulatorComponent.h"
#include "MOPSimulationBridge.h"
#include "MOPProjectileDataAsset.h"
#include "MOPTargetDataAsset.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Standalone physics engine — declarations only.
// Function bodies are compiled in MOPSimulationCore.cpp (same module, linked together).
#include "simulation.hpp"


DEFINE_LOG_CATEGORY(LogMOPSim);

// UE units (cm) per metre
static constexpr float M_TO_UE = 100.0f;
// Feet to metres
static constexpr float FT_TO_M = 0.3048f;


// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
UMOPSimulatorComponent::UMOPSimulatorComponent() {
	PrimaryComponentTick.bCanEverTick = true;

	// Pre-load Midnight Hammer / Fordow defaults so the component works
	// out of the box without any DataAsset or manual property setup.
	Scenario   = MOPBridge::GetDefaultScenario();
	Projectile = MOPBridge::GetDefaultProjectile();
	Target     = MOPBridge::GetDefaultTarget();
}


// -----------------------------------------------------------------------
// BeginPlay — auto-run simulation the moment the level starts
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::BeginPlay() {
	Super::BeginPlay();
	RunSimulation();
}


// -----------------------------------------------------------------------
// ApplyDataAssetOverrides
// If a DataAsset is assigned in the Editor, its data takes priority over
// the inline Projectile / Target properties.
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::ApplyDataAssetOverrides() {
	if (ProjectileAsset) {
		Projectile = ProjectileAsset->ProjectileData;
	}
	if (TargetAsset) {
		Target = TargetAsset->TargetData;
	}
}


// -----------------------------------------------------------------------
// RunSimulation — full synchronous MOP simulation
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::RunSimulation() {
	ApplyDataAssetOverrides();

	// 1. Convert UE structs → raw C++ structs for the physics engine
	::Projectile    RawProj     = MOPBridge::ToRaw(Projectile);
	::Target        RawTarget   = MOPBridge::ToRaw(Target);
	::ImpactScenario RawScenario = MOPBridge::ToRaw(Scenario);
	PhysicsConstants Cons;

	UE_LOG(LogMOPSim, Log, TEXT("========================================"));
	UE_LOG(LogMOPSim, Log, TEXT(" MOP SIMULATION STARTING"));
	UE_LOG(LogMOPSim, Log, TEXT(" Scenario  : %s"), *Scenario.Name);
	UE_LOG(LogMOPSim, Log, TEXT(" Projectile: %s  (%.0f kg)"), *Projectile.Name, Projectile.TotalMass);
	UE_LOG(LogMOPSim, Log, TEXT(" Target    : %s  (%d layers)"), *Target.Name, Target.Layers.Num());
	UE_LOG(LogMOPSim, Log, TEXT("========================================"));

	// 2. Run full simulation (atmospheric drop + ground penetration)
	ImpactSimulator Sim(RawProj, RawTarget, Cons);
	SimulationResult RawResult = Sim.simulate(RawScenario);

	// 3. Convert results back to UE types
	LastResult          = MOPBridge::FromRaw(RawResult);
	bSimulationComplete = true;

	// 4. Log summary
	UE_LOG(LogMOPSim, Log, TEXT("----------------------------------------"));
	UE_LOG(LogMOPSim, Log, TEXT(" SIMULATION COMPLETE"));
	UE_LOG(LogMOPSim, Log, TEXT(" Impact velocity     : %.1f m/s"), LastResult.ImpactVelocity);
	UE_LOG(LogMOPSim, Log, TEXT(" Penetration depth   : %.2f m"),   LastResult.ActualPenetrationDepth);
	UE_LOG(LogMOPSim, Log, TEXT(" Regime              : %s"),        *LastResult.Regime);
	UE_LOG(LogMOPSim, Log, TEXT(" Outcome             : %s"),        *LastResult.OutcomeSummary);
	UE_LOG(LogMOPSim, Log, TEXT(" Casing failure      : %s"),        LastResult.bCasingFailure        ? TEXT("YES") : TEXT("No"));
	UE_LOG(LogMOPSim, Log, TEXT(" Premature detonation: %s"),        LastResult.bPrematureDetonation  ? TEXT("YES") : TEXT("No"));
	UE_LOG(LogMOPSim, Log, TEXT(" Explosive survives  : %s"),        LastResult.bExplosiveChargeSurvives ? TEXT("Yes") : TEXT("NO"));
	UE_LOG(LogMOPSim, Log, TEXT(" Kinetic shock       : %.3e J"),    LastResult.KineticShockJoules);
	UE_LOG(LogMOPSim, Log, TEXT(" Explosive yield     : %.3e J"),    LastResult.TotalExplosiveYield);
	UE_LOG(LogMOPSim, Log, TEXT(" Drop frames         : %d"),         LastResult.DropFrames.Num());
	UE_LOG(LogMOPSim, Log, TEXT(" Penetration frames  : %d"),         LastResult.PenetrationFrames.Num());
	UE_LOG(LogMOPSim, Log, TEXT("----------------------------------------"));

	// 5. Log full telemetry events
	LogTelemetry(LastResult);

	// 6. Debug draw in viewport
	ImpactWorldLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
	DrawDebugVisualization(LastResult);

	// 7. Initialise tick-based playback
	ResetPlayback();
}


// -----------------------------------------------------------------------
// ClearVisualization
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::ClearVisualization() {
	UWorld* World = GetWorld();
	if (World) {
		FlushPersistentDebugLines(World);
	}
}


// -----------------------------------------------------------------------
// ResetPlayback
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::ResetPlayback() {
	PlaybackTime          = 0.0f;
	PlaybackFrameIdx      = 0;
	bInPenetrationPhase   = false;
	bPlaybackActive       = bSimulationComplete;
	PlaybackAccumX        = 0.0;
	PlaybackPrevFrameTime = 0.0;
}


// -----------------------------------------------------------------------
// DrawDebugVisualization
//
// Coordinate convention:
//   - Impact point = Owner's world location (origin for all offsets)
//   - UP  = +Z (above ground = positive altitude in UE)
//   - DOWN = -Z (underground = negative, driven by penetration depth)
//   - HORIZONTAL offset = +X (accumulated from CurrentVX integration)
//   - All metres → cm via M_TO_UE
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::DrawDebugVisualization(const FMOPSimulationResult& Result) {
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Origin = ImpactWorldLocation;
	const float   D      = DebugDrawDuration;
	const float   Scale  = FMath::Max(0.1f, DebugScale);


	// ---- Drop Trajectory --------------------------------------------
	if (bDrawDropTrajectory && Result.DropFrames.Num() > 0) {
		FVector PrevPos  = Origin + FVector(0.f, 0.f,
		                        (float)(Result.AltitudeFt * FT_TO_M * M_TO_UE));
		double  AccumX   = 0.0;
		double  PrevTime = 0.0;

		for (int32 i = 0; i < Result.DropFrames.Num(); ++i) {
			const FMOPTelemetryFrame& F = Result.DropFrames[i];

			// Integrate horizontal velocity to get lateral offset
			double dt = F.Time - PrevTime;
			AccumX   += F.CurrentVX * dt;
			PrevTime  = F.Time;

			FVector Pos = Origin + FVector(
				(float)(AccumX    * M_TO_UE),
				0.f,
				(float)(F.Altitude * M_TO_UE));

			// Sonic boom frame → yellow; normal → cyan
			FColor LineColor = F.bIsSonicBoom ? FColor::Yellow : FColor(0, 180, 255);
			DrawDebugLine(World, PrevPos, Pos, LineColor, false, D, 0, 2.5f * Scale);

			// Draw a sphere every 5 frames and on sonic boom events
			if (i % 5 == 0 || F.bIsSonicBoom) {
				float Radius = F.bIsSonicBoom ? 120.f * Scale : 50.f * Scale;
				DrawDebugSphere(World, Pos, Radius, 8, LineColor, false, D);
			}

			// Annotate sonic boom with text
			if (F.bIsSonicBoom) {
				DrawDebugString(World, Pos + FVector(0, 0, 200 * Scale),
					FString::Printf(TEXT("[SONIC BOOM] Mach %.2f | T+%.1fs"), F.Mach, F.Time),
					nullptr, FColor::Yellow, D);
			}

			PrevPos = Pos;
		}

		// Label the drop start
		FVector StartPos = Origin + FVector(0.f, 0.f,
		                        (float)(Result.AltitudeFt * FT_TO_M * M_TO_UE));
		DrawDebugString(World, StartPos + FVector(0, 0, 300 * Scale),
			FString::Printf(TEXT("DROP START\n%.0f ft | %.1f m/s | Mach %.2f"),
				Result.AltitudeFt, Result.Velocity, Result.MachNumber),
			nullptr, FColor(0, 180, 255), D);
	}


	// ---- Impact Point -----------------------------------------------
	DrawDebugSphere(World, Origin, 200.f * Scale, 16, FColor::Yellow, false, D);
	DrawDebugString(World, Origin + FVector(250 * Scale, 0, 250 * Scale),
		FString::Printf(TEXT("IMPACT\n%.1f m/s"), LastResult.ImpactVelocity),
		nullptr, FColor::Yellow, D);


	// ---- Penetration Tunnel -----------------------------------------
	if (bDrawPenetrationTunnel && Result.PenetrationFrames.Num() > 0) {
		FVector PrevPos = Origin;

		for (int32 i = 0; i < Result.PenetrationFrames.Num(); ++i) {
			const FMOPTelemetryFrame& F = Result.PenetrationFrames[i];

			// Depth is positive downward; in UE world -Z goes underground
			FVector Pos = Origin + FVector(0.f, 0.f, (float)(-F.Depth * M_TO_UE));

			// Eroding → orange; rigid penetration → green
			FColor LineColor = F.bIsEroding ? FColor(255, 140, 0) : FColor(0, 220, 80);
			DrawDebugLine(World, PrevPos, Pos, LineColor, false, D, 0, 4.f * Scale);

			// Sphere every 10 frames
			if (i % 10 == 0) {
				DrawDebugSphere(World, Pos, 40.f * Scale, 8, LineColor, false, D);
			}

			PrevPos = Pos;
		}

		// Final resting depth marker
		FVector FinalPos = Origin + FVector(0.f, 0.f,
		                        (float)(-Result.ActualPenetrationDepth * M_TO_UE));
		DrawDebugSphere(World, FinalPos, 200.f * Scale, 16, FColor::Red, false, D);
		DrawDebugString(World, FinalPos + FVector(250 * Scale, 0, 0),
			FString::Printf(TEXT("FINAL DEPTH: %.2f m\nRegime: %s\n%s"),
				Result.ActualPenetrationDepth, *Result.Regime, *Result.OutcomeSummary),
			nullptr, FColor::Red, D);
	}


	// ---- Layer Boundaries -------------------------------------------
	if (bDrawLayerBoundaries && Target.Layers.Num() > 0) {
		// Layer boundary colours cycle through white, cyan, magenta
		const FColor LayerColors[] = { FColor::White, FColor::Cyan, FColor::Magenta,
		                               FColor(255,165,0), FColor::Green };
		const int32 NumColors = UE_ARRAY_COUNT(LayerColors);

		double AccumDepth = 0.0;
		for (int32 i = 0; i < Target.Layers.Num(); ++i) {
			AccumDepth += Target.Layers[i].Thickness;
			FVector BoundaryPos = Origin + FVector(0.f, 0.f, (float)(-AccumDepth * M_TO_UE));
			FColor  BColor      = LayerColors[i % NumColors];

			// Horizontal disc at layer transition
			DrawDebugCircle(World, BoundaryPos, 600.f * Scale, 32,
			                BColor, false, D, 0, 3.f * Scale,
			                FVector(1, 0, 0), FVector(0, 1, 0));

			DrawDebugString(World, BoundaryPos + FVector(650 * Scale, 0, 0),
				FString::Printf(TEXT("Layer %d: %s\nDepth: %.1f m  | Density: %.0f kg/m³"),
					i + 1, *Target.Layers[i].MaterialName,
					AccumDepth, Target.Layers[i].Density),
				nullptr, BColor, D);
		}
	}
}


// -----------------------------------------------------------------------
// LogTelemetry — UE Output Log telemetry trace
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::LogTelemetry(const FMOPSimulationResult& Result) {
	// Drop phase: log every frame that is notable (sonic boom, first frame, last frame)
	UE_LOG(LogMOPSim, Log, TEXT("--- Atmospheric Drop Telemetry (%d frames) ---"),
	       Result.DropFrames.Num());
	for (int32 i = 0; i < Result.DropFrames.Num(); ++i) {
		const FMOPTelemetryFrame& F = Result.DropFrames[i];
		bool bNotable = (i == 0) || (i == Result.DropFrames.Num() - 1) || F.bIsSonicBoom;
		if (bNotable) {
			UE_LOG(LogMOPSim, F.bIsSonicBoom ? ELogVerbosity::Warning : ELogVerbosity::Log,
				TEXT("  [T+%.2fs] Alt: %.0f m | Vel: %.1f m/s | Mach: %.3f | Cd: %.4f%s"),
				F.Time, F.Altitude, F.Velocity, F.Mach, F.DragCoefficient,
				F.bIsSonicBoom ? TEXT("  <<< SONIC BOOM") : TEXT(""));
		}
	}

	// Penetration phase: log every 10th frame + key events
	UE_LOG(LogMOPSim, Log, TEXT("--- Ground Penetration Telemetry (%d frames) ---"),
	       Result.PenetrationFrames.Num());
	for (int32 i = 0; i < Result.PenetrationFrames.Num(); ++i) {
		const FMOPTelemetryFrame& F = Result.PenetrationFrames[i];
		bool bLog = (i % 10 == 0) || (i == Result.PenetrationFrames.Num() - 1);
		if (bLog) {
			UE_LOG(LogMOPSim, Log,
				TEXT("  [T+%.3fms] Depth: %.2f m | Vel: %.1f m/s | Mach: %.4f | Temp: %.0f K | %s"),
				F.Time * 1000.0, F.Depth, F.Velocity, F.Mach,
				F.Heat * 1800.0, // approx — heat is normalised 0-1 against melting point
				F.bIsEroding ? TEXT("[ERODING]") : TEXT("Rigid"));
		}
	}
}


// -----------------------------------------------------------------------
// TickComponent — real-time actor animation through TelemetryFrames
// -----------------------------------------------------------------------
void UMOPSimulatorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bPlaybackActive || !bSimulationComplete) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	PlaybackTime += DeltaTime;


	// ---- Phase 1: Atmospheric Drop ------------------------------------
	if (!bInPenetrationPhase) {
		const TArray<FMOPTelemetryFrame>& Frames = LastResult.DropFrames;
		if (Frames.Num() == 0) {
			bInPenetrationPhase = true;
			return;
		}

		// Advance frame index to match simulation time
		while (PlaybackFrameIdx + 1 < Frames.Num() &&
		       Frames[PlaybackFrameIdx + 1].Time <= (double)PlaybackTime) {
			PlaybackFrameIdx++;
		}

		const FMOPTelemetryFrame& Frame = Frames[PlaybackFrameIdx];

		// Re-integrate horizontal position up to this frame
		double AccumX = 0.0;
		double PrevT  = 0.0;
		for (int32 i = 0; i <= PlaybackFrameIdx; ++i) {
			double dt = Frames[i].Time - PrevT;
			AccumX   += Frames[i].CurrentVX * dt;
			PrevT     = Frames[i].Time;
		}

		FVector NewPos = ImpactWorldLocation + FVector(
			(float)(AccumX * M_TO_UE),
			0.f,
			(float)(Frame.Altitude * M_TO_UE));
		Owner->SetActorLocation(NewPos);

		// Apply pitch rotation to visually bank the bomb during drop
		double PitchDeg = Frame.PitchRad * (180.0 / 3.14159265358979);
		Owner->SetActorRotation(FRotator((float)PitchDeg, 0.f, 0.f));

		// Transition to penetration phase when drop is done
		if (PlaybackFrameIdx >= Frames.Num() - 1) {
			bInPenetrationPhase = true;
			PlaybackFrameIdx    = 0;
			PlaybackTime        = 0.0f;
			Owner->SetActorLocation(ImpactWorldLocation);
		}
	}


	// ---- Phase 2: Ground Penetration ----------------------------------
	else {
		const TArray<FMOPTelemetryFrame>& Frames = LastResult.PenetrationFrames;
		if (Frames.Num() == 0) {
			bPlaybackActive = false;
			return;
		}

		// Scale wall-clock time by TimeScalePen so the full penetration
		// event plays back in ~6 seconds (as set by the simulator).
		double TimeScalePen = FMath::Max(0.001, LastResult.TimeScalePen);
		double SimTime      = (double)PlaybackTime / TimeScalePen;

		while (PlaybackFrameIdx + 1 < Frames.Num() &&
		       Frames[PlaybackFrameIdx + 1].Time <= SimTime) {
			PlaybackFrameIdx++;
		}

		const FMOPTelemetryFrame& Frame = Frames[PlaybackFrameIdx];
		FVector NewPos = ImpactWorldLocation + FVector(0.f, 0.f,
		                     (float)(-Frame.Depth * M_TO_UE));
		Owner->SetActorLocation(NewPos);

		// Tilt actor to match obliquity
		double ObliquityDeg = Frame.ObliquityDeg;
		Owner->SetActorRotation(FRotator((float)(-90.0 + ObliquityDeg), 0.f, 0.f));

		if (PlaybackFrameIdx >= Frames.Num() - 1) {
			bPlaybackActive = false;
			UE_LOG(LogMOPSim, Log,
				TEXT("Playback complete. Final rest depth: %.2f m — %s"),
				LastResult.ActualPenetrationDepth, *LastResult.Regime);
		}
	}
}
