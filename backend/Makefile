# Copyright (c) 2026 Omid Teimory. All Rights Reserved

CXX ?= g++
CXXFLAGS ?= -std=c++23 -Wall -Wextra -O2 -Iinclude -Iinclude/penetration/versionOne

ifeq ($(OS),Windows_NT)
    EXE = .exe
else
    EXE =
endif

ifneq ($(findstring sh,$(SHELL)),)
    MKDIR_BIN = mkdir -p bin
    MKDIR_BUILD = mkdir -p build
    RM_CLEAN = rm -rf build bin 3d_visualizer.html
else
    MKDIR_BIN = if not exist bin mkdir bin
    MKDIR_BUILD = if not exist build mkdir build
    RM_CLEAN = if exist build rmdir /Q /S build & if exist bin rmdir /Q /S bin & if exist 3d_visualizer.html del /Q /F 3d_visualizer.html
endif

TARGET = bin/mop_sim$(EXE)
TEST_TARGET = bin/test_simulation$(EXE)
TEST_IMPACT_TARGET = bin/test_impactShock$(EXE)
TEST_EXPLOSIVE_TARGET = bin/test_explosiveShock$(EXE)
TEST_AERO_TARGET = bin/test_aerodynamic_accuracy$(EXE)
TEST_AI_PARITY_TARGET = bin/test_ai_vs_interactive_parity$(EXE)
TEST_PARAM_FALLBACK_TARGET = bin/test_parameter_missing_fallback$(EXE)
TEST_PEN_ACCURACY_TARGET = bin/test_penetration_accuracy$(EXE)
TEST_SCENARIO_DUP_TARGET = bin/test_scenario_duplication$(EXE)
TEST_STRESS_PERF_TARGET = bin/test_stress_performance$(EXE)
TEST_TELEMETRY_RENDER_TARGET = bin/test_telemetry_render_validation$(EXE)

OBJS = build/main.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_OBJS = build/test_simulation.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_IMPACT_OBJS = build/impactShock.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_EXPLOSIVE_OBJS = build/explosiveShock.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_AERO_OBJS = build/test_aerodynamic_accuracy.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_AI_PARITY_OBJS = build/test_ai_vs_interactive_parity.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_PARAM_FALLBACK_OBJS = build/test_parameter_missing_fallback.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_PEN_ACCURACY_OBJS = build/test_penetration_accuracy.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_SCENARIO_DUP_OBJS = build/test_scenario_duplication.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_STRESS_PERF_OBJS = build/test_stress_performance.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_TELEMETRY_RENDER_OBJS = build/test_telemetry_render_validation.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/main.o: src/simulation/penetration/versionOne/main.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp include/penetration/versionOne/telemetry_exporter.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/simulation.o: src/simulation/penetration/versionOne/simulation.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/environment_physics.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/config_loader.o: src/simulation/penetration/versionOne/config_loader.cpp include/penetration/versionOne/config_loader.hpp include/penetration/versionOne/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/environment_physics.o: src/simulation/penetration/versionOne/environment_physics.cpp include/penetration/versionOne/environment_physics.hpp include/penetration/versionOne/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/telemetry_exporter.o: src/simulation/penetration/versionOne/telemetry_exporter.cpp include/penetration/versionOne/telemetry_exporter.hpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/environment_physics.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_simulation.o: tests/versionOne/test_simulation.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/impactShock.o: tests/versionOne/impactShock.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/explosiveShock.o: tests/versionOne/explosiveShock.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_aerodynamic_accuracy.o: tests/versionOne/test_aerodynamic_accuracy.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_ai_vs_interactive_parity.o: tests/versionOne/test_ai_vs_interactive_parity.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_parameter_missing_fallback.o: tests/versionOne/test_parameter_missing_fallback.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_penetration_accuracy.o: tests/versionOne/test_penetration_accuracy.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_scenario_duplication.o: tests/versionOne/test_scenario_duplication.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_stress_performance.o: tests/versionOne/test_stress_performance.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_telemetry_render_validation.o: tests/versionOne/test_telemetry_render_validation.cpp include/penetration/versionOne/simulation.hpp include/penetration/versionOne/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TEST_TARGET): $(TEST_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_IMPACT_TARGET): $(TEST_IMPACT_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_EXPLOSIVE_TARGET): $(TEST_EXPLOSIVE_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_AERO_TARGET): $(TEST_AERO_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_AI_PARITY_TARGET): $(TEST_AI_PARITY_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_PARAM_FALLBACK_TARGET): $(TEST_PARAM_FALLBACK_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_PEN_ACCURACY_TARGET): $(TEST_PEN_ACCURACY_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_SCENARIO_DUP_TARGET): $(TEST_SCENARIO_DUP_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_STRESS_PERF_TARGET): $(TEST_STRESS_PERF_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_TELEMETRY_RENDER_TARGET): $(TEST_TELEMETRY_RENDER_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: $(TEST_TARGET) $(TEST_IMPACT_TARGET) $(TEST_EXPLOSIVE_TARGET) $(TEST_AERO_TARGET) $(TEST_AI_PARITY_TARGET) $(TEST_PARAM_FALLBACK_TARGET) $(TEST_PEN_ACCURACY_TARGET) $(TEST_SCENARIO_DUP_TARGET) $(TEST_STRESS_PERF_TARGET) $(TEST_TELEMETRY_RENDER_TARGET)
	@echo "Running all unit tests..."
	@$(TEST_TARGET)
	@$(TEST_IMPACT_TARGET)
	@$(TEST_EXPLOSIVE_TARGET)
	@$(TEST_AERO_TARGET)
	@$(TEST_AI_PARITY_TARGET)
	@$(TEST_PARAM_FALLBACK_TARGET)
	@$(TEST_PEN_ACCURACY_TARGET)
	@$(TEST_SCENARIO_DUP_TARGET)
	@$(TEST_STRESS_PERF_TARGET)
	@$(TEST_TELEMETRY_RENDER_TARGET)

test-sim: $(TEST_TARGET)
	@echo "Running simulation test suite..."
	@$(TEST_TARGET)

test-impact: $(TEST_IMPACT_TARGET)
	@echo "Running impact shockwave test..."
	@$(TEST_IMPACT_TARGET)

test-explosive: $(TEST_EXPLOSIVE_TARGET)
	@echo "Running explosive shockwave test..."
	@$(TEST_EXPLOSIVE_TARGET)

clean:
	@echo "Cleaning build artifacts..."
	@$(RM_CLEAN)

.PHONY: all test test-sim test-impact test-explosive clean

