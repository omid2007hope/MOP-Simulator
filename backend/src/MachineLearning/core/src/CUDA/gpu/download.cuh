#ifndef DOWNLOAD_CUH
#define DOWNLOAD_CUH

#include "../util/gpu_types.cuh"

#ifdef __cplusplus
extern "C" {
#endif

CUDA_API bool download_from_gpu(void* h_context,
				size_t context_elements,
				void* h_target,
				size_t target_elements,
				const GpuPortion* gpu,
				int buffer_index);

// Reads and clears the loss accumulator: *h_loss_sum receives the sum of
// 0.5 * (y - t)^2 over every training step since the previous call.
CUDA_API bool download_loss(GpuActivations* gpu_activations, float* h_loss_sum);

// Copies the latest forward-pass output (bfloat16) to the host.
CUDA_API bool download_output(const GpuActivations* gpu_activations,
			      void* h_output,
			      size_t output_elements);

// Copies the FP32 master weights and biases to the host.
CUDA_API bool download_ai_weights(const GpuWeights* gpu,
				  float* h_weights,
				  size_t weight_elements,
				  float* h_bias,
				  size_t bias_elements);

#ifdef __cplusplus
}
#endif

#endif // DOWNLOAD_CUH
