#include <cmath>
#include <iostream>
#include <vector>

#include "../include/transfer.hpp"
#include "../src/CUDA/transfer.cuh"
#include "test_header.hpp"

bool test_transfer() {
	std::cout << "\n=== Transfer Layer Test Suite ===\n";
	int s_pass = 0, s_fail = 0;

	const size_t in_dim = 512;
	const size_t out_dim = 4;
	const size_t weight_count = in_dim * out_dim;
	const size_t bias_count = out_dim;

	GpuWeights gpu_weights {};
	GpuActivations gpu_activations {};

	// Create test weights and biases
	std::vector<float> h_weights(weight_count);
	for (size_t i = 0; i < weight_count; ++i) {
		h_weights[i] = static_cast<float>(i % 100) * 0.01f - 0.5f;
	}
	std::vector<float> h_bias(bias_count);
	for (size_t i = 0; i < bias_count; ++i) {
		h_bias[i] = 0.25f * static_cast<float>(i + 1);
	}

	size_t pending_steps = 42;

	// ── [1] Test Transfer::upload_weights_to_gpu ──────────────────────────────
	std::cout << "\n[1] Upload weights to GPU & pending_steps reset\n";
	bool upload_ok = Transfer::upload_weights_to_gpu(
		h_weights, h_bias, pending_steps, gpu_weights, gpu_activations);
	test_check(upload_ok, "upload_weights_to_gpu succeeds", s_pass, s_fail);
	test_check_eq(pending_steps, size_t {0}, "pending_steps reset to 0 after upload", s_pass, s_fail);
	test_check(gpu_weights.d_weights_master != nullptr, "d_weights_master allocated on GPU", s_pass, s_fail);
	test_check(gpu_weights.d_bias_master != nullptr, "d_bias_master allocated on GPU", s_pass, s_fail);

	// ── [2] Test Transfer::download_weights_from_gpu ────────────────────────────
	std::cout << "\n[2] Download weights from GPU & bitwise accuracy\n";
	std::vector<float> downloaded_w(weight_count, 0.0f);
	std::vector<float> downloaded_b(bias_count, 0.0f);

	bool download_ok = Transfer::download_weights_from_gpu(
		downloaded_w, downloaded_b, gpu_weights);
	test_check(download_ok, "download_weights_from_gpu succeeds", s_pass, s_fail);

	bool weights_matched = true;
	for (size_t i = 0; i < weight_count; ++i) {
		if (std::abs(downloaded_w[i] - h_weights[i]) > 1e-6f) {
			weights_matched = false;
			break;
		}
	}
	test_check(weights_matched, "Downloaded weights match uploaded values exactly", s_pass, s_fail);

	bool bias_matched = true;
	for (size_t i = 0; i < bias_count; ++i) {
		if (std::abs(downloaded_b[i] - h_bias[i]) > 1e-6f) {
			bias_matched = false;
			break;
		}
	}
	test_check(bias_matched, "Downloaded biases match uploaded values exactly", s_pass, s_fail);

	// ── Cleanup ───────────────────────────────────────────────────────────────
	free_gpu_weights(&gpu_weights);
	free_gpu_activations(&gpu_activations);

	std::cout << "=== Transfer Layer Test Suite: " << s_pass << " passed, " << s_fail
		  << " failed ===\n";
	return s_fail == 0;
}
