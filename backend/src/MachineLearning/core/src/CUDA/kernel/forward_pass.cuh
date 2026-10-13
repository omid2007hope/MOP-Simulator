#ifndef FORWARD_PASS_CUH
#define FORWARD_PASS_CUH

#include "../util/gpu_types.cuh"

#ifdef __cplusplus
extern "C" {
#endif

CUDA_API bool forward_pass(const GpuPortion* gpu_portion,
			   const GpuWeights* gpu_weights,
			   GpuActivations* gpu_activations,
			   size_t input_dim,
			   size_t output_dim,
			   int buffer_index);

CUDA_API bool run_cutlass_saturation_benchmark(int M, int N, int K, int iterations,
					       double* out_tflops,
					       double* out_bandwidth_gb_s,
					       float* out_avg_ms);

#ifdef __cplusplus
}
#endif

#endif // FORWARD_PASS_CUH
