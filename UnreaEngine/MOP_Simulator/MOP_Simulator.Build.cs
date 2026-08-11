// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MOP_Simulator : ModuleRules
{
	public MOP_Simulator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// C++20 required for std::numbers::pi used in the standalone physics engine
		CppStandard = CppStandardVersion.Cpp20;

		// Expose the standalone engine headers to the UE module build:
		//   include/  — simulation.hpp, environment_physics.hpp, default.hpp
		//   src/      — environment_physics.cpp, simulation.cpp (included by MOPSimulationCore.cpp)
		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "..", "..", "include"),
			Path.Combine(ModuleDirectory, "..", "..", "src"),
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore",
			"Json", "JsonUtilities"  // for FJsonObject / FJsonSerializer (Option A fallback)
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
