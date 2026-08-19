# 4.2 UE Types, Components & Bridge

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 🧩 Unreal Engine USTRUCTs (`MOPTypes.h`)

All data structures are mirrored into Unreal Engine's reflection system (`USTRUCT(BlueprintType)`), making them accessible in Blueprints, Sequencer, and C++.

```cpp
USTRUCT(BlueprintType)
struct FMOPProjectile {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    FString Name = TEXT("GBU-57 MOP");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float Length = 6.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float Diameter = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float TotalMass = 13608.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float ExplosiveMass = 2404.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float YieldStrength = 2.1e9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float HugoniotC0 = 4570.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float HugoniotS = 1.49f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP|Projectile")
    float ExplosiveCriticalEnergy = 3.0e15f;
};
```

---

## 🔌 Header-Only Bridge (`MOPSimulationBridge.h`)

`MOPSimulationBridge.h` provides inline translation functions between Unreal Engine reflection types and raw C++ structs, guaranteeing zero runtime allocation overhead.

```cpp
namespace MOPBridge {

inline Projectile ToRaw(const FMOPProjectile& In) {
    Projectile Out;
    Out.name = TCHAR_TO_UTF8(*In.Name);
    Out.length = In.Length;
    Out.diameter = In.Diameter;
    Out.total_mass = In.TotalMass;
    Out.explosive_mass = In.ExplosiveMass;
    Out.casing_density = In.CasingDensity;
    Out.yield_strength = In.YieldStrength;
    Out.hugoniot_c0 = In.HugoniotC0;
    Out.hugoniot_s = In.HugoniotS;
    Out.explosive_critical_energy = In.ExplosiveCriticalEnergy;
    return Out;
}

inline FMOPSimulationResult FromRaw(const SimulationResult& In) {
    FMOPSimulationResult Out;
    Out.ScenarioName = UTF8_TO_TCHAR(In.scenario_name.c_str());
    Out.KineticEnergy = In.kinetic_energy;
    Out.ActualPenetrationDepth = In.actual_penetration_depth;
    Out.CasingFailure = In.casing_failure;
    Out.ExplosiveChargeSurvives = In.explosive_charge_survives;
    Out.Regime = UTF8_TO_TCHAR(In.regime.c_str());
    Out.PeakShockPressureGPa = In.shock_pressure_gpa_peak;
    return Out;
}

} // namespace MOPBridge
```

---

## 🎮 `UMOPSimulatorComponent` (`MOPSimulatorComponent.h`)

An `UActorComponent` that can be attached to any Unreal Actor (e.g. a B-2 Spirit bomber, a missile, or an impact test rig).

### Key Blueprint Callable API:

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MOP_SIMULATOR_API UMOPSimulatorComponent : public UActorComponent {
    GENERATED_BODY()

public:
    // Synchronous execution on the calling thread
    UFUNCTION(BlueprintCallable, Category = "MOP Simulation")
    FMOPSimulationResult RunSimulation(const FMOPImpactScenario& Scenario);

    // Asynchronous non-blocking execution on background worker thread
    UFUNCTION(BlueprintCallable, Category = "MOP Simulation")
    void RunSimulationAsync(const FMOPImpactScenario& Scenario);

    // Dynamic Multicast Delegate fired when background simulation completes
    UPROPERTY(BlueprintAssignable, Category = "MOP Simulation")
    FOnSimulationCompleted OnSimulationCompleted;

    // Configurable DataAssets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Setup")
    UMOPProjectileDataAsset* ProjectilePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOP Setup")
    UMOPTargetDataAsset* TargetPreset;
};
```

---

## 📦 DataAssets (`UMOPProjectileDataAsset` & `UMOPTargetDataAsset`)

Allow designers to create reusable weapon and target presets in the Unreal Content Browser without touching code:

```cpp
UCLASS(BlueprintType)
class MOP_SIMULATOR_API UMOPProjectileDataAsset : public UPrimaryDataAsset {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
    FMOPProjectile ProjectileData;
};
```

---

## 🧭 Navigation

* [Back to 4. Unreal Engine Integration](04-unreal-engine-integration.md)
* [Proceed to 5. Testing & Quality Assurance](05-testing-and-quality-assurance.md)
* [Explore 4.1 UE Module Structure & Build Configuration](04-01-ue-module-structure-and-build.md)
