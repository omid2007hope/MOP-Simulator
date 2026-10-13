#include <cuda_bf16.h>
#include <cuda_runtime.h>

#include "../util/cuda_utils.cuh"
#include "upload.cuh"

extern "C" CUDA_API bool upload_to_gpu(const void* h_context,
				       size_t context_elements,
				       const void* h_target,
				       size_t target_elements,
				       GpuPortion* gpu,
				       int buffer_index) {
	if (!gpu || !h_context || !h_target || context_elements == 0 || target_elements == 0) {
		fprintf(stderr, "Invalid memory parameters in upload_to_gpu\n");
		return false;
	}

	// ── Context Window (X) — Allocate both buffers if size changed ──────────────
	for (size_t i = 0; i < gpu->context_ptr; ++i) {
		if (gpu->context_elements != context_elements) {
			if (gpu->d_context_window[i]) {
				if (!CUDA_CHECK(cudaFree(gpu->d_context_window[i])))
					return false;
				gpu->d_context_window[i] = nullptr;
			}
			if (!CUDA_CHECK(cudaMalloc(&gpu->d_context_window[i],
						   context_elements * sizeof(__nv_bfloat16))))
				return false;
		}

		if (gpu->stream[i] == nullptr) {
			CUDA_CHECK(cudaStreamCreateWithFlags((cudaStream_t*)&gpu->stream[i],
							     cudaStreamNonBlocking));
		}

		if (gpu->compute_done[i] == nullptr) {
			if (!CUDA_CHECK(cudaEventCreateWithFlags((cudaEvent_t*)&gpu->compute_done[i],
								 cudaEventDisableTiming)))
				return false;
		}
	}
	gpu->context_elements = context_elements;

	if (!CUDA_CHECK(cudaMemcpyAsync(gpu->d_context_window[buffer_index],
					h_context,
					context_elements * sizeof(__nv_bfloat16),
					cudaMemcpyHostToDevice,
					(cudaStream_t)gpu->stream[buffer_index])))
		return false;

	// ── Target (Y) — Allocate both buffers if size changed ────────────────────
	for (size_t i = 0; i < gpu->target_ptr; ++i) {
		if (gpu->target_elements != target_elements) {
			if (gpu->d_target[i]) {
				if (!CUDA_CHECK(cudaFree(gpu->d_target[i])))
					return false;
				gpu->d_target[i] = nullptr;
			}
			if (!CUDA_CHECK(cudaMalloc(&gpu->d_target[i],
						   target_elements * sizeof(__nv_bfloat16))))
				return false;
		}
	}
	gpu->target_elements = target_elements;

	if (!CUDA_CHECK(cudaMemcpyAsync(gpu->d_target[buffer_index],
					h_target,
					target_elements * sizeof(__nv_bfloat16),
					cudaMemcpyHostToDevice,
					(cudaStream_t)gpu->stream[buffer_index])))
		return false;

	return true;
}

__global__ static void float_to_bf16_kernel(const float* __restrict__ src,
					    __nv_bfloat16* __restrict__ dst,
					    size_t n) {
	size_t i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < n)
		dst[i] = __float2bfloat16(src[i]);
}

extern "C" CUDA_API bool upload_ai_weights(const float* h_weights,
					   size_t weight_elements,
					   const float* h_bias,
					   size_t bias_elements,
					   GpuWeights* gpu) {

	if (!gpu || !h_weights || !h_bias || weight_elements == 0 || bias_elements == 0) {
		fprintf(stderr, "Invalid parameters in upload_ai_weights\n");
		return false;
	}

	// A reset can arrive mid-training: drain every in-flight kernel first so none of
	// them is still updating the weights while they are overwritten below.
	if (!CUDA_CHECK(cudaDeviceSynchronize()))
		return false;

	// ── Weights ───────────────────────────────────────────────────────────────
	if (gpu->weight_elements != weight_elements) {
		if (gpu->d_weights) {
			if (!CUDA_CHECK(cudaFree(gpu->d_weights)))
				return false;
			gpu->d_weights = nullptr;
		}
		if (gpu->d_weights_master) {
			if (!CUDA_CHECK(cudaFree(gpu->d_weights_master)))
				return false;
			gpu->d_weights_master = nullptr;
		}
		if (!CUDA_CHECK(
			    cudaMalloc(&gpu->d_weights, weight_elements * sizeof(__nv_bfloat16))))
			return false;
		if (!CUDA_CHECK(cudaMalloc(&gpu->d_weights_master, weight_elements * sizeof(float))))
			return false;
		gpu->weight_elements = weight_elements;
	}

	if (!CUDA_CHECK(cudaMemcpy(gpu->d_weights_master,
				   h_weights,
				   weight_elements * sizeof(float),
				   cudaMemcpyHostToDevice)))
		return false;

	int threads = 256;
	int blocks = static_cast<int>((weight_elements + threads - 1) / threads);
	float_to_bf16_kernel<<<blocks, threads>>>(static_cast<const float*>(gpu->d_weights_master),
						  static_cast<__nv_bfloat16*>(gpu->d_weights),
						  weight_elements);
	if (!CUDA_CHECK(cudaGetLastError()))
		return false;

	// ── Bias ──────────────────────────────────────────────────────────────────
	if (gpu->bias_elements != bias_elements) {
		if (gpu->d_bias) {
			if (!CUDA_CHECK(cudaFree(gpu->d_bias)))
				return false;
			gpu->d_bias = nullptr;
		}
		if (gpu->d_bias_master) {
			if (!CUDA_CHECK(cudaFree(gpu->d_bias_master)))
				return false;
			gpu->d_bias_master = nullptr;
		}
		if (!CUDA_CHECK(cudaMalloc(&gpu->d_bias, bias_elements * sizeof(__nv_bfloat16))))
			return false;
		if (!CUDA_CHECK(cudaMalloc(&gpu->d_bias_master, bias_elements * sizeof(float))))
			return false;
		gpu->bias_elements = bias_elements;
	}

	if (!CUDA_CHECK(cudaMemcpy(
		    gpu->d_bias_master, h_bias, bias_elements * sizeof(float), cudaMemcpyHostToDevice)))
		return false;

	blocks = static_cast<int>((bias_elements + threads - 1) / threads);
	float_to_bf16_kernel<<<blocks, threads>>>(static_cast<const float*>(gpu->d_bias_master),
						  static_cast<__nv_bfloat16*>(gpu->d_bias),
						  bias_elements);
	if (!CUDA_CHECK(cudaGetLastError()))
		return false;

	// Make the bf16 copies visible to the training streams
	return CUDA_CHECK(cudaDeviceSynchronize());
}

extern "C" CUDA_API void synchronize_stream(GpuPortion* gpu, int buffer_index) {
	if (gpu && gpu->stream[buffer_index]) {
		CUDA_CHECK(cudaStreamSynchronize((cudaStream_t)gpu->stream[buffer_index]));
	}
}
