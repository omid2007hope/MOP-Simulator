// =============================================================================
// AXIOM — test_inference.cpp
//
// Platinum-Level Comprehensive Inference & Tokenizer Test Suite
// Exhaustively verifies:
//   [1] Text Prompt Tokenization & Modality Boundary Invariants
//   [2] Autoregressive Context Window Generation & Padding Logistics
//   [3] Modality Decoders across All 5 Dimensions (Text, Num, Img, Vid, Voice)
//   [4] Nearest-Neighbor Euclidean L2 Space Invariants & Perturbation Stability
//   [5] Multi-Iteration Forward Inference Stress & Zero VRAM Leakage
//   [6] Defensive Weight File Corruption & Rejection Immunity
//   [7] Hardware Telemetry Bounds & Safe Querying
// =============================================================================

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdfloat>
#include <string>
#include <vector>

#include "../include/ai.hpp"
#include "../include/parser.hpp"
#include "../src/CUDA/util/telemetry.cuh"
#include "test_header.hpp"

bool test_inference() {
	std::cout << "\n=== Platinum Inference & System Resilience Test Suite ===\n";
	int s_pass = 0, s_fail = 0;

	const size_t in_dim = 512;
	const size_t out_dim = 4;
	const float w_min = -0.1f;
	const float w_max = 0.1f;

	Elements ai_elements {};
	AI ai(in_dim, out_dim, w_min, w_max, ai_elements);
	ai.upload_weights_to_gpu();

	Input input;
	input.embedding_dim = out_dim;
	Parser parser(true, test_embedded_path(), test_asset_dirs());

	// ── [1] Prompt Tokenization & Modality Boundary Invariants ─────────────────
	std::cout << "\n[1] Prompt Tokenization & Modality Boundary Invariants\n";

	// 1.1 Empty prompt
	std::vector<uint32_t> empty_tokens = parser.tokenize_prompt(input, "");
	test_check(empty_tokens.empty(), "Empty prompt produces 0 tokens", s_pass, s_fail);

	// 1.2 Whitespace-only prompt
	std::vector<uint32_t> ws_tokens = parser.tokenize_prompt(input, "   \t  \n  \r  ");
	test_check(ws_tokens.empty(), "Whitespace-only prompt produces 0 tokens", s_pass, s_fail);

	// 1.3 Normal prompt with irregular spacing
	std::vector<uint32_t> normal_tokens =
		parser.tokenize_prompt(input, "  The   quick\tbrown\nfox   jumps  ");
	test_check_eq(normal_tokens.size(), static_cast<size_t>(5), "Irregular whitespace tokenized to exactly 5 words", s_pass, s_fail);

	// 1.4 Repeated words must reuse identical token IDs
	std::vector<uint32_t> repeat_tokens = parser.tokenize_prompt(input, "fox fox fox");
	test_check_eq(repeat_tokens.size(), static_cast<size_t>(3), "Repeated prompt has 3 tokens", s_pass, s_fail);
	test_check(repeat_tokens[0] == repeat_tokens[1] && repeat_tokens[1] == repeat_tokens[2],
		   "Repeated words share exact identical token ID", s_pass, s_fail);

	// 1.5 Modality Boundary Check: Text IDs must strictly reside in [1, 9,999,999]
	bool text_ids_in_bounds = true;
	for (const auto& [word, id] : input.text_dictionary) {
		if (id < 1 || id >= 10000000) {
			text_ids_in_bounds = false;
			break;
		}
	}
	test_check(text_ids_in_bounds, "All allocated text IDs strictly within [1, 9,999,999]", s_pass, s_fail);

	// ── [2] Autoregressive Context Window Generation & Padding Logistics ────────
	std::cout << "\n[2] Autoregressive Context Window Generation & Padding Logistics\n";

	// 2.1 Zero tokens input -> exact 512 zeros
	std::vector<std::bfloat16_t> zero_win = parser.tokens_to_context_window(input, {}, 128);
	test_check_eq(zero_win.size(), static_cast<size_t>(512), "Zero tokens produces exact 512 elements", s_pass, s_fail);
	bool all_zero = true;
	for (auto val : zero_win) {
		if (static_cast<float>(val) != 0.0f) {
			all_zero = false;
			break;
		}
	}
	test_check(all_zero, "Zero-token context window is 100% zero-padded", s_pass, s_fail);

	// 2.2 Single token input -> padded with 127 zero tokens (508 floats) and 1 token at end
	std::vector<uint32_t> single_tok = {normal_tokens[0]};
	std::vector<std::bfloat16_t> single_win =
		parser.tokens_to_context_window(input, single_tok, 128);
	test_check_eq(single_win.size(), static_cast<size_t>(512), "Single token produces exact 512 elements", s_pass, s_fail);
	bool prefix_zero = true;
	for (size_t i = 0; i < 508; ++i) {
		if (static_cast<float>(single_win[i]) != 0.0f) {
			prefix_zero = false;
			break;
		}
	}
	test_check(prefix_zero, "Single token has 508 leading zero floats (127 padded tokens)", s_pass, s_fail);
	const auto& expected_tok_emb = input.embedding_table[single_tok[0]];
	bool suffix_matches = true;
	for (size_t d = 0; d < 4; ++d) {
		if (single_win[508 + d] != expected_tok_emb[d]) {
			suffix_matches = false;
			break;
		}
	}
	test_check(suffix_matches, "Single token embedding correctly placed at tail of context window", s_pass, s_fail);

	// 2.3 Overflow tokens (>128 tokens) -> slices strictly the latest 128 tokens
	std::vector<uint32_t> large_tokens(200);
	for (size_t i = 0; i < 200; ++i) {
		large_tokens[i] = normal_tokens[i % normal_tokens.size()];
	}
	std::vector<std::bfloat16_t> large_win =
		parser.tokens_to_context_window(input, large_tokens, 128);
	test_check_eq(large_win.size(), static_cast<size_t>(512), "Overflow context window maintains exact 512 elements", s_pass, s_fail);

	// The very first token of the 128-window must correspond to large_tokens[200 - 128 = 72]
	uint32_t expected_first_slice_tok = large_tokens[72];
	const auto& first_slice_emb = input.embedding_table[expected_first_slice_tok];
	bool slice_start_matches = true;
	for (size_t d = 0; d < 4; ++d) {
		if (large_win[d] != first_slice_emb[d]) {
			slice_start_matches = false;
			break;
		}
	}
	test_check(slice_start_matches, "Overflow window correctly slices the most recent 128 tokens", s_pass, s_fail);

	// ── [3] Modality Decoders across All 5 Dimensions ───────────────────────────
	std::cout << "\n[3] Modality Decoders across All 5 Dimensions\n";

	// 3.1 Text Modality
	input.text_dictionary["Axiom"] = 42;
	input.reverse_text_dictionary[42] = "Axiom";
	test_check_eq(parser.token_to_string(input, 42), std::string("Axiom"), "Text token #42 decodes to 'Axiom'", s_pass, s_fail);
	test_check_eq(parser.token_to_string(input, 999), std::string("[Text #999]"), "Unmapped text ID decodes to '[Text #999]'", s_pass, s_fail);

	// 3.2 Number Modality (10,000,000 to 19,999,999)
	input.number_dictionary[3.14159] = 10000005;
	test_check(parser.token_to_string(input, 10000005).find("3.14159") != std::string::npos,
		   "Number token #10000005 decodes to numeric string 3.14159", s_pass, s_fail);
	test_check_eq(parser.token_to_string(input, 10000099), std::string("[Number #10000099]"),
		      "Unmapped number ID decodes to '[Number #10000099]'", s_pass, s_fail);

	// 3.3 Image Modality (20,000,000 to 29,999,999)
	input.image_dictionary[static_cast<uint8_t>(255)] = 20000010;
	test_check_eq(parser.token_to_string(input, 20000010), std::string("[Image byte: 255]"),
		      "Image token #20000010 decodes to '[Image byte: 255]'", s_pass, s_fail);

	// 3.4 Video Modality (30,000,000 to 39,999,999)
	input.video_dictionary[static_cast<uint8_t>(128)] = 30000020;
	test_check_eq(parser.token_to_string(input, 30000020), std::string("[Video byte: 128]"),
		      "Video token #30000020 decodes to '[Video byte: 128]'", s_pass, s_fail);

	// 3.5 Voice Modality (40,000,000 to 49,999,999)
	input.voice_dictionary[static_cast<uint8_t>(64)] = 40000030;
	test_check_eq(parser.token_to_string(input, 40000030), std::string("[Voice byte: 64]"),
		      "Voice token #40000030 decodes to '[Voice byte: 64]'", s_pass, s_fail);

	// 3.6 Unknown / Out-of-bounds Modality (>= 50,000,000)
	test_check_eq(parser.token_to_string(input, 99999999), std::string("[Unknown #99999999]"),
		      "Token ID >= 50,000,000 decodes to '[Unknown #99999999]'", s_pass, s_fail);

	// ── [4] Nearest-Neighbor Euclidean L2 Space Invariants ───────────────────────
	std::cout << "\n[4] Nearest-Neighbor Euclidean L2 Space Invariants\n";

	parser.build_embedding_table(input);

	// 4.1 Exact Match on Known Token
	uint32_t target_tok = 42; // "Axiom"
	const auto& target_emb = input.embedding_table[target_tok];
	std::vector<float> target_vec(out_dim);
	for (size_t d = 0; d < out_dim; ++d) {
		target_vec[d] = static_cast<float>(target_emb[d]);
	}
	uint32_t decoded_tok = parser.decode_embedding_to_token(input, target_vec);
	test_check_eq(decoded_tok, target_tok, "Exact embedding vector decodes to exact token ID #42", s_pass, s_fail);

	// 4.2 Distance Monotonicity under Noise Perturbation
	std::vector<float> noisy_vec = target_vec;
	noisy_vec[0] += 0.005f;
	noisy_vec[1] -= 0.005f;
	noisy_vec[2] += 0.002f;
	noisy_vec[3] -= 0.001f;
	uint32_t noisy_tok = parser.decode_embedding_to_token(input, noisy_vec);
	test_check_eq(noisy_tok, target_tok, "Perturbed vector robustly clusters to nearest neighbor #42", s_pass, s_fail);

	// 4.3 Empty Vocabulary Safety Guard
	Input empty_input;
	empty_input.embedding_dim = 4;
	uint32_t empty_tok = parser.decode_embedding_to_token(empty_input, target_vec);
	test_check_eq(empty_tok, static_cast<uint32_t>(0), "Empty vocabulary safely returns token ID 0", s_pass, s_fail);
	test_check_eq(parser.decode_prediction(empty_input, target_vec), std::string("[Empty Vocabulary]"),
		      "Empty vocabulary safely decodes to '[Empty Vocabulary]'", s_pass, s_fail);

	// ── [5] Multi-Iteration Forward Inference Stress & Zero VRAM Leakage ────────
	std::cout << "\n[5] Multi-Iteration Forward Inference Stress & Zero VRAM Leakage\n";

	size_t vram_free_start = 0, vram_total_start = 0;
	get_gpu_telemetry(&vram_free_start, &vram_total_start);

	const size_t STRESS_PASSES = 100;
	bool all_inference_finite = true;
	bool all_inference_non_negative = true;

	for (size_t iter = 0; iter < STRESS_PASSES; ++iter) {
		std::vector<std::bfloat16_t> iter_ctx(in_dim);
		for (size_t i = 0; i < in_dim; ++i) {
			iter_ctx[i] = static_cast<std::bfloat16_t>(
				std::sin(static_cast<double>(iter * in_dim + i)));
		}

		std::vector<float> pred;
		if (!ai.predict(iter_ctx, pred)) {
			all_inference_finite = false;
			break;
		}

		if (pred.size() != out_dim) {
			all_inference_finite = false;
			break;
		}

		for (float val : pred) {
			if (std::isnan(val) || std::isinf(val)) {
				all_inference_finite = false;
			}
			if (val < 0.0f) {
				all_inference_non_negative = false;
			}
		}
	}

	test_check(all_inference_finite, "All 100 consecutive forward inference outputs are finite numbers", s_pass, s_fail);
	test_check(all_inference_non_negative, "All 100 outputs are strictly >= 0 (CUTLASS ReLU non-negativity)", s_pass, s_fail);

	size_t vram_free_end = 0, vram_total_end = 0;
	get_gpu_telemetry(&vram_free_end, &vram_total_end);

	// VRAM must not decrease over the 100 iterations (zero memory leakage)
	test_check(vram_free_end >= vram_free_start || (vram_free_start - vram_free_end) < 1024 * 1024,
		   "Zero VRAM leakage detected across 100 consecutive Tensor Core inferences", s_pass, s_fail);

	// ── [6] Defensive Weight File Corruption & Rejection Immunity ───────────────
	std::cout << "\n[6] Defensive Weight File Corruption & Rejection Immunity\n";

	const std::string valid_weights_file = "test_valid_weights.axiom";
	test_check(ai.save_weights(valid_weights_file), "ai.save_weights() creates baseline valid checkpoint", s_pass, s_fail);

	// Snapshot of working memory
	std::vector<float> baseline_weights = ai.h_weights;
	std::vector<float> baseline_bias = ai.h_bias;

	// 6.1 Sub-40-byte truncated header file
	{
		std::string trunc_path = "test_trunc_header.axiom";
		std::ofstream out(trunc_path, std::ios::binary);
		char short_bytes[20] = {'A', 'X', 'I', 'O', 'M'};
		out.write(short_bytes, sizeof(short_bytes));
		out.close();
		test_check(!ai.load_weights(trunc_path), "load_weights() rejects sub-40-byte truncated file", s_pass, s_fail);
		std::filesystem::remove(trunc_path);
	}

	// 6.2 Header with dimension mismatch
	{
		std::string dim_mismatch_path = "test_dim_mismatch.axiom";
		std::ofstream out(dim_mismatch_path, std::ios::binary);
		AxiomWeightHeader bad_hdr {};
		bad_hdr.in_dim = 1024; // Mismatch with model's in_dim 512
		bad_hdr.out_dim = 4;
		bad_hdr.weight_count = 1024 * 4;
		bad_hdr.bias_count = 4;
		out.write(reinterpret_cast<const char*>(&bad_hdr), sizeof(bad_hdr));
		out.close();
		test_check(!ai.load_weights(dim_mismatch_path), "load_weights() rejects header with in_dim mismatch (1024 != 512)", s_pass, s_fail);
		std::filesystem::remove(dim_mismatch_path);
	}

	// 6.3 Valid header but truncated weight payload (< 8248 bytes)
	{
		std::string trunc_payload_path = "test_trunc_payload.axiom";
		std::ofstream out(trunc_payload_path, std::ios::binary);
		AxiomWeightHeader hdr {};
		hdr.in_dim = in_dim;
		hdr.out_dim = out_dim;
		hdr.weight_count = in_dim * out_dim;
		hdr.bias_count = out_dim;
		out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
		float half_weights[100] = {0.0f};
		out.write(reinterpret_cast<const char*>(half_weights), sizeof(half_weights));
		out.close();
		test_check(!ai.load_weights(trunc_payload_path), "load_weights() rejects truncated payload data", s_pass, s_fail);
		std::filesystem::remove(trunc_payload_path);
	}

	// 6.4 State preservation assertion: Failed loads must NOT mutate active brain
	test_check(ai.h_weights == baseline_weights, "State immunity: Host weights unchanged after failed load attempts", s_pass, s_fail);
	test_check(ai.h_bias == baseline_bias, "State immunity: Host biases unchanged after failed load attempts", s_pass, s_fail);

	std::filesystem::remove(valid_weights_file);

	// ── [7] Hardware Telemetry Bounds & Safe Querying ───────────────────────────
	std::cout << "\n[7] Hardware Telemetry Bounds & Safe Querying\n";

	size_t gpu_free = 0, gpu_total = 0;
	get_gpu_telemetry(&gpu_free, &gpu_total);
	test_check(gpu_total > 0, "GPU total memory reported is non-zero (> 0 MB)", s_pass, s_fail);
	test_check(gpu_free > 0, "GPU free memory reported is non-zero (> 0 MB)", s_pass, s_fail);
	test_check(gpu_free <= gpu_total, "GPU free memory is strictly <= total memory", s_pass, s_fail);

	std::cout << "=== Platinum Inference & System Resilience Test Suite: " << s_pass << " passed, "
		  << s_fail << " failed ===\n";
	return s_fail == 0;
}
