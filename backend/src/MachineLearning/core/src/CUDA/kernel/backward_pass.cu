#include <cuda_bf16.h>
#include <cuda_runtime.h>

#include "../util/cuda_utils.cuh"
#include "backward_pass.cuh"

// =============================================================================
// Backward pass: loss gradient + SGD update, one thread per weight.
//
//   loss      L  = 0.5 * sum_i (y_i - t_i)^2
//   dL/dy_i      = y_i - t_i
//   dL/dz_i      = dL/dy_i * [y_i > 0]            (ReLU gradient, z = Wx + b)
//   dL/dW_ij     = dL/dz_i * x_j                  (outer product, K = 1)
//   dL/db_i      = dL/dz_i
//
// dL/dW is a rank-1 outer product: every weight needs exactly one multiply, so there
// is nothing for the Tensor Cores to accelerate. A flat one-thread-per-weight kernel
// already saturates memory bandwidth, which is the limit here.
//
// The update runs on the FP32 master copies and the result is rounded into the bf16
// copies that the forward pass reads.
// =============================================================================
__global__ static void backward_pass_kernel(float* __restrict__ W_master,
					    __nv_bfloat16* __restrict__ W,
					    float* __restrict__ b_master,
					    __nv_bfloat16* __restrict__ b,
					    const __nv_bfloat16* __restrict__ x,
					    const __nv_bfloat16* __restrict__ y,
					    const __nv_bfloat16* __restrict__ target,
					    float* __restrict__ loss,
					    size_t in_dim,
					    size_t out_dim,
					    float learning_rate) {

	/*
  blockDim.x is the number of apartments in each building (e.g., 10 apartments
  per building).

  blockIdx.x is the building number (e.g., Building #3).


  threadIdx.x is the specific apartment number inside that building (e.g.,
  Apartment #5).

  Your Global ID is your absolute position on the street: (Building 3 × 10
  apartments) + Apartment 5 = The 35th apartment overall.
*/

	/*
  Here the Global ID is the index of one weight (row * in_dim + col), so
  blockIdx.x is no longer constant: out_dim * in_dim = 2048 weights need 8
  blocks of 256 threads.
*/

	size_t i = blockIdx.x * (size_t)blockDim.x + threadIdx.x;

	if (i < out_dim * in_dim) {
		size_t row = i / in_dim;
		size_t col = i % in_dim;

		float y_val = __bfloat162float(y[row]);
		float t_val = __bfloat162float(target[row]);
		float dy = y_val - t_val;

		// Gradient of ReLU
		float dz = (y_val > 0.0f) ? dy : 0.0f;

		float w_val = W_master[i] - learning_rate * dz * __bfloat162float(x[col]);
		W_master[i] = w_val;
		W[i] = __float2bfloat16(w_val);

		// Each row's first thread owns that row's bias and its share of the loss
		if (col == 0) {
			float b_val = b_master[row] - learning_rate * dz;
			b_master[row] = b_val;
			b[row] = __float2bfloat16(b_val);

			atomicAdd(loss, 0.5f * dy * dy);
		}
	}
}

extern "C" CUDA_API bool backward_pass(const GpuPortion* gpu_portion,
				       GpuWeights* gpu_weights,
				       const GpuActivations* gpu_activations,
				       size_t input_dim,
				       size_t output_dim,
				       float learning_rate,
				       int buffer_index) {

	// Safe Guard
	if (!gpu_portion || !gpu_weights || !gpu_activations ||
	    !gpu_portion->d_context_window[buffer_index] || !gpu_portion->d_target[buffer_index] ||
	    !gpu_weights->d_weights || !gpu_weights->d_bias || !gpu_weights->d_weights_master ||
	    !gpu_weights->d_bias_master || !gpu_activations->d_output || !gpu_activations->d_loss ||
	    input_dim == 0 || output_dim == 0) {
		return false;
	}

	cudaStream_t stream = (cudaStream_t)gpu_portion->stream[buffer_index];

	Grid grid(input_dim * output_dim);

	backward_pass_kernel<<<grid.block, grid.thread, grid.index, stream>>>(
		static_cast<float*>(gpu_weights->d_weights_master),
		static_cast<__nv_bfloat16*>(gpu_weights->d_weights),
		static_cast<float*>(gpu_weights->d_bias_master),
		static_cast<__nv_bfloat16*>(gpu_weights->d_bias),
		static_cast<const __nv_bfloat16*>(gpu_portion->d_context_window[buffer_index]),
		static_cast<const __nv_bfloat16*>(gpu_activations->d_output),
		static_cast<const __nv_bfloat16*>(gpu_portion->d_target[buffer_index]),
		static_cast<float*>(gpu_activations->d_loss),
		input_dim,
		output_dim,
		learning_rate);

	if (!CUDA_CHECK(cudaGetLastError()))
		return false;

	// Releases the next step's forward pass (see forward_pass)
	return CUDA_CHECK(cudaEventRecord((cudaEvent_t)gpu_portion->compute_done[buffer_index], stream));
}
