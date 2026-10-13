#ifndef BACKWARD_PASS_CUH
#define BACKWARD_PASS_CUH

#include "../util/gpu_types.cuh"

#ifdef __cplusplus
extern "C" {
#endif

CUDA_API bool backward_pass(const GpuPortion* gpu_portion,
			    GpuWeights* gpu_weights,
			    const GpuActivations* gpu_activations,
			    size_t input_dim,
			    size_t output_dim,
			    float learning_rate,
			    int buffer_index);

#ifdef __cplusplus
}
#endif

#endif // BACKWARD_PASS_CUH
