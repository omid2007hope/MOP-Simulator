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

TARGET = bin/sim$(EXE)
TEST_TARGET = bin/test_simulation$(EXE)

OBJS = build/main.o build/simulation.o build/config_loader.o
TEST_OBJS = build/test_simulation.o build/simulation.o build/config_loader.o

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/main.o: src/main.cpp include/simulation.hpp include/config_loader.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/simulation.o: src/simulation.cpp include/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/config_loader.o: src/config_loader.cpp include/config_loader.hpp include/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/test_simulation.o: tests/test_simulation.cpp include/simulation.hpp
	@$(MKDIR_BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test: $(TEST_TARGET)
	@echo "Running unit tests..."
	@$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	@$(MKDIR_BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	@$(RM_CLEAN)

.PHONY: all test clean
