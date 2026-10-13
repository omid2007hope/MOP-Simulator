#include <cuda_bf16.h>
#include <cuda_runtime.h>

#include "../util/cuda_utils.cuh"
#include "download.cuh"

extern "C" CUDA_API bool download_from_gpu(void* h_context,
					   size_t context_elements,
					   void* h_target,
					   size_t target_elements,
					   const GpuPortion* gpu,
					   int buffer_index) {
	if (!gpu)
		return false;

	if (h_context && gpu->d_context_window[buffer_index] &&
	    context_elements == gpu->context_elements) {
		if (!CUDA_CHECK(cudaMemcpyAsync(h_context,
						gpu->d_context_window[buffer_index],
						context_elements * sizeof(__nv_bfloat16),
						cudaMemcpyDeviceToHost,
						(cudaStream_t)gpu->stream[buffer_index])))
			return false;
	}

	if (h_target && gpu->d_target[buffer_index] && target_elements == gpu->target_elements) {
		if (!CUDA_CHECK(cudaMemcpyAsync(h_target,
						gpu->d_target[buffer_index],
						target_elements * sizeof(__nv_bfloat16),
						cudaMemcpyDeviceToHost,
						(cudaStream_t)gpu->stream[buffer_index])))
			return false;
	}

	if (gpu->stream[buffer_index]) {
		if (!CUDA_CHECK(cudaStreamSynchronize((cudaStream_t)gpu->stream[buffer_index])))
			return false;
	}

	return true;
}

// The functions below are diagnostics (called once per report interval, never per
// step), so each one simply drains the device before reading.

extern "C" CUDA_API bool download_loss(GpuActivations* gpu_activations, float* h_loss_sum) {
	if (!gpu_activations || !h_loss_sum)
		return false;

	*h_loss_sum = 0.0f;

	// Nothing has been trained yet
	if (!gpu_activations->d_loss)
		return true;

	if (!CUDA_CHECK(cudaDeviceSynchronize()))
		return false;

	if (!CUDA_CHECK(cudaMemcpy(
		    h_loss_sum, gpu_activations->d_loss, sizeof(float), cudaMemcpyDeviceToHost)))
		return false;

	// Start the next interval from zero
	return CUDA_CHECK(cudaMemset(gpu_activations->d_loss, 0, sizeof(float)));
}

extern "C" CUDA_API bool download_output(const GpuActivations* gpu_activations,
					 void* h_output,
					 size_t output_elements) {
	if (!gpu_activations || !h_output || !gpu_activations->d_output ||
	    output_elements != gpu_activations->output_elements)
		return false;

	if (!CUDA_CHECK(cudaDeviceSynchronize()))
		return false;

	return CUDA_CHECK(cudaMemcpy(h_output,
				     gpu_activations->d_output,
				     output_elements * sizeof(__nv_bfloat16),
				     cudaMemcpyDeviceToHost));
}

extern "C" CUDA_API bool download_ai_weights(const GpuWeights* gpu,
					     float* h_weights,
					     size_t weight_elements,
					     float* h_bias,
					     size_t bias_elements) {
	if (!gpu || !h_weights || !h_bias || !gpu->d_weights_master || !gpu->d_bias_master ||
	    weight_elements != gpu->weight_elements || bias_elements != gpu->bias_elements)
		return false;

	if (!CUDA_CHECK(cudaDeviceSynchronize()))
		return false;

	// The FP32 master copies are the true weights; the bf16 ones are just their rounding
	if (!CUDA_CHECK(cudaMemcpy(h_weights,
				   gpu->d_weights_master,
				   weight_elements * sizeof(float),
				   cudaMemcpyDeviceToHost)))
		return false;

	return CUDA_CHECK(cudaMemcpy(
		h_bias, gpu->d_bias_master, bias_elements * sizeof(float), cudaMemcpyDeviceToHost));
}
