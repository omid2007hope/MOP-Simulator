#ifndef include_transfer
#define include_transfer

#include <cstddef>
#include <vector>

#include "../src/CUDA/transfer.cuh"


class Transfer {
public:
	// Method to move memory to GPU
	static bool upload_weights_to_gpu(const std::vector<float>& h_weights,
					  const std::vector<float>& h_bias,
					  size_t& pending_steps,
					  GpuWeights& gpu_weights,
					  GpuActivations& gpu_activations);

	// Method to copy the trained weights back into h_weights / h_bias
	static bool download_weights_from_gpu(std::vector<float>& h_weights,
					      std::vector<float>& h_bias,
					      const GpuWeights& gpu_weights);
};

#endif