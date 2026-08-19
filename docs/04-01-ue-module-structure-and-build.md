# 4.1 UE Module Structure & Build Configuration

// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

---

## 📁 Module Directory Structure

The Unreal Engine integration module is located in `backend/UnreaEngine/` and structured as a standard Unreal C++ module:

```text
backend/UnreaEngine/
├── MOP_Simulator.Target.cs           # Standalone Game Target Build Rules
├── MOP_SimulatorEditor.Target.cs     # Editor Target Build Rules
└── MOP_Simulator/
    ├── MOP_Simulator.Build.cs        # Module Dependency & Include Rules
    ├── MOP_Simulator.h               # Module Header
    ├── MOP_Simulator.cpp             # Primary Module Implementation
    ├── MOP_SimulatorGameModeBase.h   # Base GameMode Header
    ├── MOP_SimulatorGameModeBase.cpp # Base GameMode Source
    ├── MOPTypes.h                    # Unreal Engine USTRUCT Definitions
    ├── MOPSimulationBridge.h         # Inline C++ Bridge Translator
    ├── MOPSimulatorComponent.h       # ActorComponent Header
    ├── MOPSimulatorComponent.cpp     # ActorComponent Source
    ├── MOPProjectileDataAsset.h      # DataAsset for Munitions
    ├── MOPTargetDataAsset.h          # DataAsset for Targets
    ├── MOPPhysicsLibrary.h           # Blueprint Function Library Header
    └── MOPPhysicsLibrary.cpp         # Blueprint Function Library Source
```

---

## 🛠️ Build Configuration (`MOP_Simulator.Build.cs`)

The `Build.cs` file declares Unreal Build Tool (UBT) module dependencies and configures include paths to access the standalone C++ physics headers:

```csharp
using UnrealBuildTool;
using System.IO;

public class MOP_Simulator : ModuleRules
{
    public MOP_Simulator(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20; // Or Cpp23 if supported by compiler

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "PhysicsCore"
        });

        // Add include path to standalone C++ physics headers
        string IncludePath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../include/penetration"));
        PublicIncludePaths.Add(IncludePath);
    }
}
```

---

## 🎯 Target Build Rules (`MOP_Simulator.Target.cs`)

Defines target settings for building the standalone game client and editor binaries:

```csharp
using UnrealBuildTool;
using System.Collections.Generic;

public class MOP_SimulatorTarget : TargetRules
{
    public MOP_SimulatorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V4;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("MOP_Simulator");
    }
}
```

---

## 🔨 Compilation Instructions

1. Copy the `backend/UnreaEngine/` folder into your Unreal Engine project's `Source/` directory.
2. Ensure the relative path in `MOP_Simulator.Build.cs` points to `backend/include/penetration/`.
3. Right-click your `.uproject` file and select **Generate Visual Studio project files**.
4. Open the generated `.sln` in Visual Studio 2022 or Rider.
5. Select **Development Editor** configuration and compile the project (`Ctrl + Shift + B`).

---

## 🧭 Navigation

* [Back to 4. Unreal Engine Integration](04-unreal-engine-integration.md)
* [Proceed to 4.2 UE Types, Components & Bridge](04-02-ue-types-components-and-bridge.md)
* [Explore 2.1 C++ Physics Engine](02-01-cpp-physics-engine.md)
