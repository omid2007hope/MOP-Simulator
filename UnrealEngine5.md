# MOP Simulation Integration Plan

## 1. Overview

The goal is to migrate the standalone C++ MOP physics simulation (`unimplanted/`) into Unreal Engine 5 (`Source/MOP/`), transforming it from a batch-processed command-line tool into an integrated, real-time capable Unreal Engine system.

## 2. Core Data Structures (`simulation.hpp`)

Convert standard C++ structs into `USTRUCT(BlueprintType)` to make them accessible within UE5 Blueprints.

- **Types Transformation:**
  - `std::string` ➔ `FString`
  - `std::vector<T>` ➔ `TArray<T>`
  - `double` ➔ `double` (UE5 supports double precision, which is critical for physics/math fidelity).
- **Structs to Migrate:**
  - `FPhysicsConstants`, `FImpactScenario`, `FAtmosphereState`, `FAircraft`, `FProjectile`, `FTargetLayer`, `FTarget`, `FTelemetryFrame`, `FSimulationResult`.
  - Add `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MOP Simulation")` to fields for Editor visibility.

## 3. Physics & Math Utilities (`environment_physics.hpp`)

Refactor the `EnvironmentPhysics` namespace into a `UBlueprintFunctionLibrary` called `UMOPEnvironmentPhysicsLibrary`.

- This allows atmospheric and physics calculations to be called from both C++ and Blueprints.
- Use `FMath` for math operations instead of `<cmath>` where applicable for Unreal standard conformity.

## 4. Simulator Engine (`simulation.hpp` & `.cpp`)

Adapt the `ImpactSimulator` class into an Unreal Actor Component: `UMOPImpactSimulatorComponent`.

- **Current State:** The simulator runs a while-loop and computes the entire trajectory and penetration in one go, returning a list of `TelemetryFrame`s.
- **UE5 Integration:**
  - **Pre-calculation (Recommended for parity):** Keep the logic synchronous. Call `Simulate()` and get the final result immediately. Then use the generated `TelemetryFrame`s to animate the munition actor over time in the engine.
  - **Tick-based:** Alternatively, refactor the simulation loops to run inside the component's `TickComponent`.

## 5. Configuration Loader (`config_loader.hpp`)

The current system uses `nlohmann/json` to load scenarios, projectiles, and targets from JSON files.

- **UE5 Adaptation:**
  - **Option A:** Replace `nlohmann/json` with Unreal's `FJsonObject` and `FJsonSerializer` to continue loading external JSON.
  - **Option B (Recommended):** Create `UDataAsset` classes (`UMOPProjectileDataAsset`, `UMOPTargetDataAsset`) so users can create and edit configurations directly in the Unreal Editor instead of writing JSON files.

## 6. Telemetry Exporter (`telemetry_exporter.hpp`)

- Replace standard console output (`std::cout`) with `UE_LOG(LogTemp, Log, TEXT(...))`.
- The 3D ASCII visualizer and HTML exporter can be kept for debugging, replacing `std::ofstream` with `FFileHelper::SaveStringToFile`.
- **In-Engine Visualization:** Use `DrawDebugLine`, `DrawDebugSphere`, and `DrawDebugString` to visualize the telemetry path (trajectory and underground crater profile) directly in the Unreal Editor viewport.
