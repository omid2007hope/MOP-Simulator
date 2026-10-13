CXX      = g++
CXXFLAGS = -Wall -std=c++23 -Iinclude

# ─── CUDA ────────────────────────────────────────────────────────────────────
CUDA_DIR  = C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v13.3
CUDA_LIB_DIR = $(CUDA_DIR)/lib/x64
NVCC      = "$(CUDA_DIR)/bin/nvcc.exe"
# sm_89 = RTX 4080 Laptop (Ada Lovelace, compute capability 8.9)
# -ccbin points to MSVC compiler (cl.exe) which is strictly required by nvcc on Windows
NVCCFLAGS = -arch=sm_89 --std=c++17 -ccbin "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64" \
            -I"$(CUDA_DIR)/include" -Ithird_party/cutlass/include --expt-relaxed-constexpr \
            -Xcompiler "/Zc:preprocessor,/Zc:__cplusplus"

CUDA_SRC_DIR = src/CUDA
CUDA_SRCS    = $(wildcard $(CUDA_SRC_DIR)/gpu/*.cu) \
               $(wildcard $(CUDA_SRC_DIR)/kernel/*.cu) \
               $(wildcard $(CUDA_SRC_DIR)/util/*.cu)

# ─── CPU sources ─────────────────────────────────────────────────────────────
SRC_DIR  = src
TEST_DIR = test
UTIL_DIR = util

UTIL_SRCS = $(wildcard $(UTIL_DIR)/*.cpp)
ALL_SRCS  = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(TEST_DIR)/*.cpp) $(UTIL_SRCS)
APP_SRCS  = $(ALL_SRCS)
TEST_SRCS = $(ALL_SRCS)

# ─── Output targets ──────────────────────────────────────────────────────────
APP_TARGET  = app.exe
TEST_TARGET = test.exe

# ─── Default: compile everything ─────────────────────────────────────────────
all: $(APP_TARGET) $(TEST_TARGET)

# ─── Compile CUDA sources into a single DLL ──────────────────────────────────
# Build a shared library so MSVC's linker handles all MSVC-specific symbols.
CUDA_DLL = cuda_transfer.dll
$(CUDA_DLL): $(CUDA_SRCS)
	-$(NVCC) $(NVCCFLAGS) -DBUILDING_CUDA_DLL -shared $(CUDA_SRCS) -o $(CUDA_DLL)

# Helper: return the CUDA DLL only if it was successfully built
CUDA_LINK = $(if $(wildcard $(CUDA_DLL)),$(CUDA_DLL),)

# ─── Link CPU + GPU into the app ─────────────────────────────────────────────
$(APP_TARGET): $(APP_SRCS) $(CUDA_DLL)
	$(CXX) $(CXXFLAGS) $(APP_SRCS) $(CUDA_LINK) -o $(APP_TARGET) -lpsapi

# ─── Link CPU + GPU into the test binary ─────────────────────────────────────
$(TEST_TARGET): $(TEST_SRCS) $(CUDA_DLL)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) $(CUDA_LINK) -o $(TEST_TARGET) -lpsapi

# ─── Clean ───────────────────────────────────────────────────────────────────
clean:
	-rm -f $(APP_TARGET) $(TEST_TARGET) $(CUDA_DLL) cuda_transfer.lib cuda_transfer.exp *.o *.obj *.exp *.lib *.tmp test_*.exe test_*.axiom corrupt_*.axiom weights.axiom vocab.axiom database/*.bin database/*.axiom database/*.tmp database/models/*.axiom database/models/*.bin database/models/*.tmp ../database/*.bin ../database/*.axiom ../database/*.tmp ../database/models/*.axiom ../database/models/*.bin ../database/models/*.tmp

# ─── Run tests ───────────────────────────────────────────────────────────────
test: $(TEST_TARGET)
	./$(TEST_TARGET) --test
