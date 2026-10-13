#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdfloat>
#include <string>
#include <vector>

#include "../include/ai.hpp"
#include "../include/optimizer.hpp"
#include "../include/parser.hpp"
#include "test_header.hpp"

bool test_ai() {
	std::cout << "\n=== AI Engine Test Suite ===\n";
	int s_pass = 0, s_fail = 0;

	const size_t in_dim = 512;
	const size_t out_dim = 4;
	const float w_min = -0.5f;
	const float w_max = 0.5f;

	Elements e {};

	// ── [1] Initialization & Dimensions ───────────────────────────────────────
	std::cout << "\n[1] AI Initialization & Dimensions\n";
	AI ai(in_dim, out_dim, w_min, w_max, e);

	test_check_eq(e.weight_elements, in_dim * out_dim, "Elements weight_elements == 2048", s_pass, s_fail);
	test_check_eq(e.bias_elements, out_dim, "Elements bias_elements == 4", s_pass, s_fail);
	test_check_eq(ai.h_weights.size(), in_dim * out_dim, "h_weights size == 2048", s_pass, s_fail);
	test_check_eq(ai.h_bias.size(), out_dim, "h_bias size == 4", s_pass, s_fail);

	// ── [2] Chaos / Symmetry Breaking ─────────────────────────────────────────
	std::cout << "\n[2] Weight Distribution & Chaos Verification\n";
	bool weights_in_bounds = true;
	float weight_sum = 0.0f;
	for (float w : ai.h_weights) {
		if (w < w_min || w > w_max) {
			weights_in_bounds = false;
			break;
		}
		weight_sum += w;
	}
	test_check(weights_in_bounds, "All weights bounded strictly in [w_min, w_max]", s_pass, s_fail);
	test_check(std::abs(weight_sum) > 0.001f, "Weights are non-zero chaotic distribution", s_pass, s_fail);

	bool biases_zero = true;
	for (float b : ai.h_bias) {
		if (b != 0.0f) {
			biases_zero = false;
			break;
		}
	}
	test_check(biases_zero, "Initial biases all set to 0.0f", s_pass, s_fail);

	// ── [3] GPU Transfer Roundtrip ────────────────────────────────────────────
	std::cout << "\n[3] AI GPU Transfer Roundtrip\n";
	test_check(ai.upload_weights_to_gpu(), "upload_weights_to_gpu() returns true", s_pass, s_fail);

	float original_w0 = ai.h_weights[0];
	ai.h_weights[0] = 999.0f; // Tamper with CPU cache

	test_check(ai.download_weights_from_gpu(), "download_weights_from_gpu() returns true", s_pass, s_fail);
	test_check(std::abs(ai.h_weights[0] - original_w0) < 1e-6f, "Downloaded weights restored original VRAM value", s_pass, s_fail);

	// ── [4] Loss Reading with Zero Steps ──────────────────────────────────────
	std::cout << "\n[4] Zero Pending Steps Handling\n";
	double zero_loss = 0.0;
	size_t zero_steps = 0;
	test_check(ai.read_loss(zero_loss, zero_steps), "read_loss() succeeds when pending_steps is 0", s_pass, s_fail);
	test_check_eq(zero_steps, size_t {0}, "steps reported is 0", s_pass, s_fail);
	test_check_eq(zero_loss, 0.0, "loss reported is 0.0", s_pass, s_fail);

	// ── [5] Training Loop Step & Loss Reading ─────────────────────────────────
	std::cout << "\n[5] Training Step Execution\n";
	const size_t RAM_BUF = 1000;
	const size_t TOKEN_DIM = 4;
	const size_t CTX_WIN = 128; // 512 floats window, 4 floats target (matches in_dim)
	Portion portion(RAM_BUF, TOKEN_DIM, CTX_WIN);
	portion.allocate_pinned();

	// Populate ram buffer with synthetic tokens
	portion.ram_buffer_tokens.resize(RAM_BUF);
	for (size_t i = 0; i < RAM_BUF; ++i) {
		portion.ram_buffer_tokens[i] = static_cast<std::bfloat16_t>(0.1f * ((i % 10) + 1));
	}

	Optimizer optimizer(test_embedded_path(), 0.01f);

	TrainStatus status = ai.train(optimizer, portion);
	test_check(status == TrainStatus::Trained, "ai.train() returns TrainStatus::Trained on populated buffer", s_pass, s_fail);

	double train_loss = 0.0;
	size_t train_steps = 0;
	test_check(ai.read_loss(train_loss, train_steps), "read_loss() after train() succeeds", s_pass, s_fail);
	test_check_eq(train_steps, size_t {1}, "read_loss() reports 1 pending step", s_pass, s_fail);
	test_check(train_loss > 0.0, "Training loss is strictly positive", s_pass, s_fail);

	portion.free_pinned();

	// ── [6] Buffer Exhaustion (TrainStatus::NoData) ───────────────────────────
	std::cout << "\n[6] Buffer Exhaustion Handling\n";
	Portion empty_portion(RAM_BUF, TOKEN_DIM, CTX_WIN);
	empty_portion.allocate_pinned();
	// ram_buffer_tokens is empty
	TrainStatus empty_status = ai.train(optimizer, empty_portion);
	test_check(empty_status == TrainStatus::NoData, "ai.train() returns TrainStatus::NoData when buffer is empty", s_pass, s_fail);
	empty_portion.free_pinned();

	// ── [7] Multi-Step Training Convergence ───────────────────────────────────
	std::cout << "\n[7] Multi-Step Convergence\n";
	Portion conv_portion(RAM_BUF, TOKEN_DIM, CTX_WIN);
	conv_portion.allocate_pinned();

	// Initialize weights and biases to positive regime to ensure active gradient flow (avoid dead ReLU)
	for (float& w : ai.h_weights) {
		w = 0.01f;
	}
	for (float& b : ai.h_bias) {
		b = 0.1f;
	}
	ai.upload_weights_to_gpu();

	auto refill = [&]() {
		conv_portion.ram_buffer_tokens.resize(RAM_BUF);
		for (size_t i = 0; i < RAM_BUF; ++i) {
			conv_portion.ram_buffer_tokens[i] = static_cast<std::bfloat16_t>(0.1f * ((i % 4) + 1));
		}
	};
	refill();

	// Step 1: initial loss
	ai.train(optimizer, conv_portion);
	double initial_loss = 0.0;
	size_t initial_steps = 0;
	ai.read_loss(initial_loss, initial_steps);

	// Train 50 steps on this repeatable pattern
	for (int step = 0; step < 50; ++step) {
		if (ai.train(optimizer, conv_portion) == TrainStatus::NoData) {
			refill();
			ai.train(optimizer, conv_portion);
		}
	}

	// Drain any remaining pending loss
	double drain_loss = 0.0;
	size_t drain_steps = 0;
	ai.read_loss(drain_loss, drain_steps);

	// Single evaluation step on the known pattern
	conv_portion.offset = 0;
	ai.train(optimizer, conv_portion);
	double final_loss = 0.0;
	size_t final_steps = 0;
	ai.read_loss(final_loss, final_steps);

	std::cout << "  Initial step loss: " << initial_loss << " -> Multi-step loss: " << final_loss << "\n";
	test_check(final_loss < initial_loss, "Loss decreased after training (AI is actively learning)", s_pass, s_fail);

	conv_portion.free_pinned();

	// ── [8] Weight Persistence Roundtrip (Save & Load) ────────────────────────
	std::cout << "\n[8] Weight Persistence Roundtrip (Save & Load)\n";
	const std::string test_weights_path = "test_weights.axiom";

	// Save trained weights to disk
	test_check(ai.save_weights(test_weights_path), "ai.save_weights() returns true", s_pass, s_fail);
	test_check(std::filesystem::exists(test_weights_path), "test_weights.axiom file exists on disk", s_pass, s_fail);

	const size_t expected_file_size = sizeof(AxiomWeightHeader) +
					  (in_dim * out_dim * sizeof(float)) +
					  (out_dim * sizeof(float));
	test_check_eq(std::filesystem::file_size(test_weights_path), expected_file_size, "File size matches exact binary format", s_pass, s_fail);

	// Snapshot of the saved parameters
	std::vector<float> saved_weights = ai.h_weights;
	std::vector<float> saved_bias = ai.h_bias;

	// Reset memory to chaos (breaking current state)
	ai.reset_weights(e);
	test_check(ai.upload_weights_to_gpu(), "Upload chaotic weights to GPU VRAM", s_pass, s_fail);

	// Verify host weights are different from saved
	bool weights_differ_from_saved = (ai.h_weights != saved_weights);
	test_check(weights_differ_from_saved, "Memory wiped: current weights differ from saved snapshot", s_pass, s_fail);

	// Load weights from file
	test_check(ai.load_weights(test_weights_path), "ai.load_weights() returns true", s_pass, s_fail);

	// Assert bit-for-bit equality on host memory
	test_check(ai.h_weights == saved_weights, "Loaded weights match saved weights bit-for-bit", s_pass, s_fail);
	test_check(ai.h_bias == saved_bias, "Loaded biases match saved biases bit-for-bit", s_pass, s_fail);

	// Verify GPU VRAM was also restored by load_weights()
	ai.h_weights[0] = 999.0f; // Tamper with CPU memory
	test_check(ai.download_weights_from_gpu(), "download_weights_from_gpu() succeeds after load", s_pass, s_fail);
	test_check(ai.h_weights == saved_weights, "VRAM verified: weights downloaded from GPU match saved bit-for-bit", s_pass, s_fail);

	// Test error cases: non-existent file and invalid magic header
	test_check(!ai.load_weights("non_existent_file.axiom"), "load_weights() fails on non-existent file", s_pass, s_fail);

	// Corrupt magic header test
	{
		std::string corrupt_path = "corrupt_weights.axiom";
		std::ofstream corrupt_out(corrupt_path, std::ios::binary);
		char bad_header[40] = {0};
		corrupt_out.write(bad_header, sizeof(bad_header));
		corrupt_out.close();
		test_check(!ai.load_weights(corrupt_path), "load_weights() rejects invalid magic header", s_pass, s_fail);
		std::filesystem::remove(corrupt_path);
	}

	// Clean up test file
	std::filesystem::remove(test_weights_path);

	// Nested directory creation and persistence test
	{
		const std::string nested_path = "test_artifacts/nested/weights.axiom";
		test_check(ai.save_weights(nested_path), "ai.save_weights() creates parent directories and saves atomically", s_pass, s_fail);
		test_check(std::filesystem::exists(nested_path), "Nested weight file exists on disk", s_pass, s_fail);
		test_check(ai.load_weights(nested_path), "ai.load_weights() successfully loads from nested directory", s_pass, s_fail);
		std::error_code rm_ec;
		std::filesystem::remove_all("test_artifacts", rm_ec);
	}

	// ── [9] Forward Inference & Prediction Verification ───────────────────────
	std::cout << "\n[9] Forward Inference & Prediction Verification\n";

	// 1. Predict from direct std::vector<std::bfloat16_t> context window
	std::vector<std::bfloat16_t> test_context(in_dim);
	for (size_t i = 0; i < in_dim; ++i) {
		test_context[i] = static_cast<std::bfloat16_t>(0.1f * (i % 5));
	}

	std::vector<float> prediction_output;
	test_check(ai.predict(test_context, prediction_output), "ai.predict(context_vector) returns true", s_pass, s_fail);
	test_check_eq(prediction_output.size(), out_dim, "Prediction output dimension == out_dim (4)", s_pass, s_fail);

	bool prediction_valid = true;
	for (float val : prediction_output) {
		if (std::isnan(val) || std::isinf(val)) {
			prediction_valid = false;
			break;
		}
	}
	test_check(prediction_valid, "Prediction output values are valid finite floats", s_pass, s_fail);

	// 2. Parser Vocabulary & Nearest-Neighbor Decoding Verification
	Input parser_input;
	parser_input.embedding_dim = out_dim;
	parser_input.text_dictionary["The"] = 1;
	parser_input.text_dictionary["quick"] = 2;
	parser_input.text_dictionary["brown"] = 3;
	parser_input.text_dictionary["fox"] = 4;
	parser_input.next_text_id = 5;

	Parser test_parser(true, "database/test_embedded.bin", {});
	test_parser.build_embedding_table(parser_input);

	test_check(parser_input.embedding_table.size() >= 4, "Parser embedding table populated for vocabulary", s_pass, s_fail);

	// Verify exact match decoding
	const auto& fox_emb = parser_input.embedding_table[4];
	std::vector<float> fox_float_vec(out_dim);
	for (size_t d = 0; d < out_dim; ++d) {
		fox_float_vec[d] = static_cast<float>(fox_emb[d]);
	}
	uint32_t decoded_id = test_parser.decode_embedding_to_token(parser_input, fox_float_vec);
	test_check_eq(decoded_id, static_cast<uint32_t>(4), "Exact embedding vector decodes to token ID #4 ('fox')", s_pass, s_fail);

	std::string decoded_str = test_parser.decode_prediction(parser_input, fox_float_vec);
	test_check_eq(decoded_str, std::string("fox"), "Decoded string matches 'fox'", s_pass, s_fail);

	// Verify nearest-neighbor with minor noise perturbation
	std::vector<float> noisy_fox_vec = fox_float_vec;
	noisy_fox_vec[0] += 0.001f;
	noisy_fox_vec[1] -= 0.001f;
	uint32_t noisy_decoded_id = test_parser.decode_embedding_to_token(parser_input, noisy_fox_vec);
	test_check_eq(noisy_decoded_id, static_cast<uint32_t>(4), "Perturbed embedding robustly decodes to nearest token ID #4", s_pass, s_fail);

	// Verify decoding model's live prediction
	std::string live_prediction_word = test_parser.decode_prediction(parser_input, prediction_output);
	test_check(!live_prediction_word.empty(), "Live prediction decodes to non-empty vocabulary word", s_pass, s_fail);

	// 3. Prompt Tokenization & Context Window Generation
	std::vector<uint32_t> prompt_tokens = test_parser.tokenize_prompt(parser_input, "The quick brown fox jumps");
	test_check_eq(prompt_tokens.size(), static_cast<size_t>(5), "tokenize_prompt parses 5 tokens", s_pass, s_fail);
	test_check_eq(parser_input.text_dictionary["jumps"], static_cast<uint32_t>(5), "New word 'jumps' assigned token ID #5", s_pass, s_fail);

	std::vector<std::bfloat16_t> prompt_ctx = test_parser.tokens_to_context_window(parser_input, prompt_tokens, 128);
	test_check_eq(prompt_ctx.size(), static_cast<size_t>(512), "tokens_to_context_window generates exact 512 bfloat16_t elements", s_pass, s_fail);

	std::vector<float> prompt_pred;
	test_check(ai.predict(prompt_ctx, prompt_pred), "ai.predict(prompt_ctx) succeeds on user prompt context", s_pass, s_fail);
	test_check_eq(prompt_pred.size(), out_dim, "Prompt prediction dimension == 4", s_pass, s_fail);

	std::cout << "=== AI Engine Test Suite: " << s_pass << " passed, " << s_fail
		  << " failed ===\n";
	return s_fail == 0;
}
