#include <cassert>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "../include/parser.hpp"
#include "test_header.hpp"

// ─── Minimal assertion helper ────────────────────────────────────────────────
static int s_pass = 0, s_fail = 0;

static void check(bool cond, const char* label) {
	if (cond) {
		std::cout << "  PASS [" << label << "]\n";
		++s_pass;
	} else {
		std::cerr << "  FAIL [" << label << "]\n";
		++s_fail;
	}
}

template <typename A, typename B>
static void check_eq(A actual, B expected, const char* label) {
	if (static_cast<B>(actual) == expected) {
		std::cout << "  PASS [" << label << "]: " << actual << "\n";
		++s_pass;
	} else {
		std::cerr << "  FAIL [" << label << "]: got " << actual << ", expected " << expected
			  << "\n";
		++s_fail;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
bool test_embeddings() {
	std::cout << "\n=== Embedding Test Suite ===\n";

	Input input;
	Test test = make_test_data();
	Parser parser(true, test_embedded_path(), test_asset_dirs());



	// ── [1] Tokenization produces non-zero output ────────────────────────────
	std::cout << "\n[1] Tokenization\n";
	std::vector<uint32_t> tokens = parser.tokenization(input, test);
	check(tokens.size() > 0, "tokenized_data non-empty");
	check_eq(
		tokens.size(), input.tokenized_data.size(), "return value == input.tokenized_data");

	// Text: 9 words, numbers: 12 values, image/video/voice from sample files
	check(input.text_dictionary.size() > 0, "text_dictionary populated");
	check(input.number_dictionary.size() > 0, "number_dictionary populated");

	// Token ID ranges must not overlap between modalities
	uint32_t max_text = 0;
	for (auto& [k, v] : input.text_dictionary)
		max_text = std::max(max_text, v);
	uint32_t min_number = UINT32_MAX;
	for (auto& [k, v] : input.number_dictionary)
		min_number = std::min(min_number, v);
	check(max_text < min_number, "text IDs < number IDs (no overlap)");

	// ── [2] Embedding returns correct size ───────────────────────────────────
	std::cout << "\n[2] Embedding size\n";
	std::vector<std::bfloat16_t> emb = parser.embedding(input, test);
	size_t expected_size = input.tokenized_data.size() * input.embedding_dim;
	check_eq(emb.size(), expected_size, "embedded.size == tokens * embedding_dim");
	check_eq(input.embedded_data.size(), expected_size, "input.embedded_data synced");

	std::cout << "\n=== Embedded Data Test Result ===\n";
	std::cout << "Successfully generated " << input.embedded_data.size() << " elements!\n";
	std::cout << "First 1000: ";
	for (size_t i = 0; i < input.embedded_data.size() && i < 1000; ++i) {
		std::cout << input.embedded_data[i] << " ";
	}
	std::cout << "\n";

	// ── [3] Embedding table has one entry per unique token ───────────────────
	std::cout << "\n[3] Embedding table\n";
	size_t unique_tokens = input.text_dictionary.size() + input.number_dictionary.size() +
			       input.image_dictionary.size() + input.video_dictionary.size() +
			       input.voice_dictionary.size();
	// embedding_table must be <= unique_tokens (only seen tokens get an entry)
	check(input.embedding_table.size() <= unique_tokens,
	      "embedding_table entries <= unique token count");
	check(input.embedding_table.size() > 0, "embedding_table non-empty");

	// Each entry must have exactly embedding_dim values
	bool all_dim_correct = true;
	for (auto& [id, vec] : input.embedding_table) {
		if (vec.size() != input.embedding_dim) {
			all_dim_correct = false;
			break;
		}
	}
	check(all_dim_correct, "every embedding vector has embedding_dim elements");

	// ── [4] BF16 precision: values are in [-1, 1] and non-zero ──────────────
	std::cout << "\n[4] BF16 value range\n";
	bool all_bounded = true, any_nonzero = false;
	for (auto v : emb) {
		double d = static_cast<double>(v);
		if (d < -1.5 || d > 1.5) {
			all_bounded = false;
		}
		if (d != 0.0) {
			any_nonzero = true;
		}
	}
	check(all_bounded, "all BF16 values in [-1.5, 1.5] (sin-based init)");
	check(any_nonzero, "at least one non-zero embedding value");

	// ── [5] BF16 type size ───────────────────────────────────────────────────
	std::cout << "\n[5] BF16 type size\n";
	check_eq(sizeof(std::bfloat16_t), size_t {2}, "sizeof(bfloat16_t) == 2 bytes");

	// ── [6] embedded_data.bin written to disk ───────────────────────────────
	std::cout << "\n[6] Disk output\n";
	std::filesystem::path bin = test_embedded_path();
	check(std::filesystem::exists(bin), "database/embedded_data.bin exists");
	if (std::filesystem::exists(bin)) {
		size_t file_bytes = std::filesystem::file_size(bin);
		size_t expected_bytes = expected_size * sizeof(std::bfloat16_t);
		check_eq(file_bytes,
			 expected_bytes,
			 "file size == embedded.size * sizeof(bfloat16_t)");
	}

	// ── [7] Re-run embedding is idempotent ───────────────────────────────────
	std::cout << "\n[7] Idempotency\n";
	Input input2;
	Test test2 = make_test_data();
	Parser parser2(true, test_embedded_path(), test_asset_dirs());
	parser2.tokenization(input2, test2);
	std::vector<std::bfloat16_t> emb2 = parser2.embedding(input2, test2);
	check_eq(emb2.size(), emb.size(), "second run produces same size");
	bool same_values = (emb.size() == emb2.size());
	for (size_t i = 0; i < std::min(emb.size(), emb2.size()); ++i) {
		if (emb[i] != emb2[i]) {
			same_values = false;
			break;
		}
	}
	check(same_values, "deterministic: same embedding values on re-run");

	std::cout << "\n=== Embedding Test Suite: " << s_pass << " passed, " << s_fail
		  << " failed ===\n";
	int fails = s_fail;
	s_pass = s_fail = 0; // reset for next suite
	return fails == 0;
}
