// Copyright (c) 2026 Omid Teimory. All Rights Reserved.
//
// Compiles the standalone MOP physics engine into the Unreal Engine module.
// Both environment_physics.cpp and simulation.cpp are included here as a
// single translation unit so their internal calls resolve within the same TU.
//
// std::cout output from the standalone engine goes to stdout (system console).
// High-level results are also sent to UE Output Log via UE_LOG in
// MOPSimulatorComponent.cpp.

#include "MOP_Simulator.h"

// Standalone C++ physics engine — compile directly into this TU.
// Build.cs adds "../../include" and "../../src" to include search paths,
// so relative includes inside these files resolve correctly.
#include "environment_physics.cpp"
#include "simulation.cpp"
