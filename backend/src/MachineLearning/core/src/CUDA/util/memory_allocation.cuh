#ifndef MEMORY_ALLOCATION_CUH
#define MEMORY_ALLOCATION_CUH

#include "gpu_types.cuh"

#ifdef __cplusplus
extern "C" {
#endif

CUDA_API void free_gpu(GpuPortion* gpu);
CUDA_API void free_gpu_weights(GpuWeights* gpu);
CUDA_API void free_gpu_activations(GpuActivations* gpu);

CUDA_API void* pinned_alloc(size_t bytes);
CUDA_API void pinned_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // MEMORY_ALLOCATION_CUH
