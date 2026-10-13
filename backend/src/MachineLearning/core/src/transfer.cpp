#include "../include/transfer.hpp"

#include "../src/CUDA/transfer.cuh"




// **********
// Upload
// **********

bool Transfer::upload_weights_to_gpu(const std::vector<float>& h_weights,
				     const std::vector<float>& h_bias,
				     size_t& pending_steps,
				     GpuWeights& gpu_weights,
				     GpuActivations& gpu_activations) {
	// Pass the data down to the CUDA C-interface
	if (!upload_ai_weights(h_weights.data(),
			       h_weights.size(),
			       h_bias.data(),
			       h_bias.size(),
			       &gpu_weights)) {
		return false;
	}

	// Whatever loss was measured against the previous weights no longer describes
	// this network, so start a fresh measurement.
	float stale_loss = 0.0f;
	download_loss(&gpu_activations, &stale_loss);
	pending_steps = 0;

	return true;
}

// **********
// Download
// **********

bool Transfer::download_weights_from_gpu(std::vector<float>& h_weights,
					 std::vector<float>& h_bias,
					 const GpuWeights& gpu_weights) {
	return download_ai_weights(
		&gpu_weights, h_weights.data(), h_weights.size(), h_bias.data(), h_bias.size());
}