#ifndef CUDA_UTILS_CUH
#define CUDA_UTILS_CUH

#include <cstdio>
#include <cuda_runtime.h>

inline static bool cuda_check(cudaError_t err, const char* file, int line) {
	if (err != cudaSuccess) {
		fprintf(stderr, "CUDA error at %s:%d — %s\n", file, line, cudaGetErrorString(err));
		return false;
	}
	return true;
}

#define CUDA_CHECK(call) cuda_check((call), __FILE__, __LINE__)

#endif // CUDA_UTILS_CUH
