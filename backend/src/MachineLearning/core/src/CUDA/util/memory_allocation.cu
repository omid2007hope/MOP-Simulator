#include <cstdio>
#include <cuda_runtime.h>

#include "cuda_utils.cuh"
#include "memory_allocation.cuh"

extern "C" CUDA_API void free_gpu(GpuPortion* gpu) {
	if (!gpu)
		return;
	for (size_t i = 0; i < gpu->context_ptr; ++i) {
		if (gpu->d_context_window[i]) {
			cudaFree(gpu->d_context_window[i]);
			gpu->d_context_window[i] = nullptr;
		}
	}
	gpu->context_elements = 0;

	for (size_t i = 0; i < gpu->target_ptr; ++i) {
		if (gpu->d_target[i]) {
			cudaFree(gpu->d_target[i]);
			gpu->d_target[i] = nullptr;
		}
	}
	gpu->target_elements = 0;

	for (size_t i = 0; i < gpu->context_ptr; ++i) {
		if (gpu->stream[i]) {
			cudaStreamDestroy((cudaStream_t)gpu->stream[i]);
			gpu->stream[i] = nullptr;
		}
		if (gpu->compute_done[i]) {
			cudaEventDestroy((cudaEvent_t)gpu->compute_done[i]);
			gpu->compute_done[i] = nullptr;
		}
	}
}

extern "C" CUDA_API void free_gpu_weights(GpuWeights* gpu) {
	if (!gpu)
		return;
	if (gpu->d_weights) {
		cudaFree(gpu->d_weights);
		gpu->d_weights = nullptr;
	}
	if (gpu->d_bias) {
		cudaFree(gpu->d_bias);
		gpu->d_bias = nullptr;
	}
	if (gpu->d_weights_master) {
		cudaFree(gpu->d_weights_master);
		gpu->d_weights_master = nullptr;
	}
	if (gpu->d_bias_master) {
		cudaFree(gpu->d_bias_master);
		gpu->d_bias_master = nullptr;
	}
	gpu->weight_elements = 0;
	gpu->bias_elements = 0;
}

extern "C" CUDA_API void free_gpu_activations(GpuActivations* gpu) {
	if (!gpu)
		return;
	if (gpu->d_output) {
		cudaFree(gpu->d_output);
		gpu->d_output = nullptr;
	}
	if (gpu->d_loss) {
		cudaFree(gpu->d_loss);
		gpu->d_loss = nullptr;
	}
	gpu->output_elements = 0;
}

extern "C" CUDA_API void* pinned_alloc(size_t bytes) {
	void* ptr = nullptr;
	cudaError_t err = cudaHostAlloc(&ptr, bytes, cudaHostAllocWriteCombined);
	if (err != cudaSuccess) {
		fprintf(stderr,
			"[CUDA] pinned_alloc failed (%zu bytes): %s\n",
			bytes,
			cudaGetErrorString(err));
		return nullptr;
	}
	return ptr;
}

extern "C" CUDA_API void pinned_free(void* ptr) {
	if (ptr)
		cudaFreeHost(ptr);
}
