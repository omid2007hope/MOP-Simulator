#ifndef UPLOAD_CUH
#define UPLOAD_CUH

#include "../util/gpu_types.cuh"

#ifdef __cplusplus
extern "C" {
#endif

CUDA_API bool upload_to_gpu(const void* h_context,
			    size_t context_elements,
			    const void* h_target,
			    size_t target_elements,
			    GpuPortion* gpu,
			    int buffer_index);

CUDA_API void synchronize_stream(GpuPortion* gpu, int buffer_index);

CUDA_API bool upload_ai_weights(const float* h_weights,
				size_t weight_elements,
				const float* h_bias,
				size_t bias_elements,
				GpuWeights* gpu);

#ifdef __cplusplus
}
#endif

#endif // UPLOAD_CUH
