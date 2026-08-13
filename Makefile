# Copyright (c) 2026 Omid Teimory. All Rights Reserved

CXX ?= g++
CXXFLAGS ?= -std=c++23 -Wall -Wextra -O2 -Iinclude

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

OBJS = build/main.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_OBJS = build/test_simulation.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_IMPACT_OBJS = build/impactShock.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o
TEST_EXPLOSIVE_OBJS = build/explosiveShock.o build/simulation.o build/config_loader.o build/environment_physics.o build/telemetry_exporter.o

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/main.o: src/simulation/main.cpp include/simulation.hpp include/config_loader.hpp include/default.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/simulation.o: src/simulation/simulation.cpp include/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/config_loader.o: src/simulation/config_loader.cpp include/config_loader.hpp include/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/environment_physics.o: src/simulation/environment_physics.cpp include/environment_physics.hpp include/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/telemetry_exporter.o: src/simulation/telemetry_exporter.cpp include/telemetry_exporter.hpp include/simulation.hpp include/environment_physics.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_simulation.o: tests/test_simulation.cpp include/simulation.hpp include/default.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/impactShock.o: tests/impactShock.cpp include/simulation.hpp include/default.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/explosiveShock.o: tests/explosiveShock.cpp include/simulation.hpp include/default.hpp
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

test: $(TEST_TARGET) $(TEST_IMPACT_TARGET) $(TEST_EXPLOSIVE_TARGET)
	@echo "Running all unit tests..."
	@$(TEST_TARGET)
	@$(TEST_IMPACT_TARGET)
	@$(TEST_EXPLOSIVE_TARGET)

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
