#include "../include/parser.hpp"
#include "test_header.hpp"

bool test_tokenization() {
	std::cout << "\n=== Tokenization & Modality Test Suite ===\n";
	int s_pass = 0, s_fail = 0;

	Input input;
	Test test = make_test_data();
	Parser parser(true, test_embedded_path(), test_asset_dirs());

	std::vector<uint32_t> tokenized = parser.tokenization(input, test);
	test_check(!tokenized.empty(), "Tokenized stream is non-empty", s_pass, s_fail);
	test_check_eq(tokenized.size(), input.tokenized_data.size(), "Parser output matches input.tokenized_data", s_pass, s_fail);

	// Modality dictionaries must be populated
	test_check(!input.text_dictionary.empty(), "Text vocabulary populated", s_pass, s_fail);
	test_check(!input.number_dictionary.empty(), "Number vocabulary populated", s_pass, s_fail);
	test_check(!input.image_dictionary.empty(), "Image vocabulary populated", s_pass, s_fail);
	test_check(!input.video_dictionary.empty(), "Video vocabulary populated", s_pass, s_fail);
	test_check(!input.voice_dictionary.empty(), "Voice vocabulary populated", s_pass, s_fail);

	// Generate and verify embeddings
	std::vector<std::bfloat16_t> embedded = parser.embedding(input, test);
	test_check(!embedded.empty(), "Embedded vector is non-empty", s_pass, s_fail);
	test_check_eq(embedded.size(), tokenized.size() * input.embedding_dim, "Embedded floats == tokens * embedding_dim", s_pass, s_fail);
	test_check(!input.embedding_table.empty(), "Embedding lookup table is populated", s_pass, s_fail);

	// Bulk directory tokenization verification
	Input bulk_input;
	bulk_input.embedding_dim = input.embedding_dim;
	Parser bulk_parser(false, test_embedded_path(), test_asset_dirs());
	size_t files_processed = bulk_parser.bulk_tokenize_directory(test_asset_dirs()[0], bulk_input);
	test_check(files_processed > 0, "bulk_tokenize_directory processes files in asset_sample", s_pass, s_fail);
	test_check(!bulk_input.tokenized_data.empty(), "bulk_tokenize_directory produces non-empty token stream", s_pass, s_fail);
	test_check(!bulk_input.image_dictionary.empty(), "bulk_tokenize_directory populates image namespace", s_pass, s_fail);
	test_check(!bulk_input.video_dictionary.empty(), "bulk_tokenize_directory populates video namespace", s_pass, s_fail);
	test_check(!bulk_input.voice_dictionary.empty(), "bulk_tokenize_directory populates voice namespace", s_pass, s_fail);
	test_check_eq(bulk_parser.get_last_bulk_files().size(), files_processed, "bulk_parser tracks all processed bulk files", s_pass, s_fail);

	// Test archive exclusion: files placed in archive/ are ignored
	std::error_code ec;
	const std::filesystem::path test_archive_dir = test_asset_dirs()[0] / "archive";
	std::filesystem::create_directories(test_archive_dir, ec);
	const std::filesystem::path dummy_archive_file = test_archive_dir / "ignored_dummy.txt";
	{
		std::ofstream dummy_out(dummy_archive_file);
		dummy_out << "archived content should be ignored by bulk tokenizer";
	}
	Input archive_test_input;
	archive_test_input.embedding_dim = input.embedding_dim;
	size_t archive_test_processed = bulk_parser.bulk_tokenize_directory(test_asset_dirs()[0], archive_test_input);
	test_check_eq(archive_test_processed, files_processed, "bulk_tokenize_directory strictly ignores archive folder", s_pass, s_fail);
	std::filesystem::remove(dummy_archive_file, ec);
	std::filesystem::remove(test_archive_dir, ec);

	// Non-existent directory handling
	Input missing_input;
	size_t missing_processed = bulk_parser.bulk_tokenize_directory("non_existent_folder_xyz", missing_input);
	test_check_eq(missing_processed, static_cast<size_t>(0), "bulk_tokenize_directory safely returns 0 on missing dir", s_pass, s_fail);

	// Vocabulary serialization & persistence roundtrip tests
	const std::filesystem::path test_vocab_file = "test_serialization.vocab";
	test_check(parser.save_vocabulary(test_vocab_file, input), "parser.save_vocabulary() succeeds", s_pass, s_fail);
	test_check(std::filesystem::exists(test_vocab_file), "Serialized vocabulary file exists on disk", s_pass, s_fail);

	Input restored_input;
	restored_input.embedding_dim = input.embedding_dim;
	test_check(parser.load_vocabulary(test_vocab_file, restored_input), "parser.load_vocabulary() succeeds", s_pass, s_fail);
	test_check_eq(restored_input.text_dictionary.size(), input.text_dictionary.size(), "Loaded text_dictionary size matches original", s_pass, s_fail);
	test_check_eq(restored_input.reverse_text_dictionary.size(), input.text_dictionary.size(), "reverse_text_dictionary populated with all tokens", s_pass, s_fail);

	bool all_tokens_match = true;
	for (const auto& [word, id] : input.text_dictionary) {
		auto it = restored_input.text_dictionary.find(word);
		if (it == restored_input.text_dictionary.end() || it->second != id) {
			all_tokens_match = false;
			break;
		}
		auto rev_it = restored_input.reverse_text_dictionary.find(id);
		if (rev_it == restored_input.reverse_text_dictionary.end() || rev_it->second != word) {
			all_tokens_match = false;
			break;
		}
	}
	test_check(all_tokens_match, "Every token and reverse mapping restored bit-for-bit", s_pass, s_fail);
	test_check_eq(restored_input.embedding_table.size(), restored_input.text_dictionary.size(), "Embedding table automatically reconstructed on load", s_pass, s_fail);

	// Rejection of invalid magic bytes
	const std::filesystem::path corrupt_vocab_file = "test_corrupt.vocab";
	{
		std::ofstream corrupt_out(corrupt_vocab_file, std::ios::binary);
		corrupt_out.write("BADK", 4);
		uint64_t dummy_sz = 10;
		corrupt_out.write(reinterpret_cast<const char*>(&dummy_sz), sizeof(uint64_t));
	}
	Input corrupt_input;
	test_check(!parser.load_vocabulary(corrupt_vocab_file, corrupt_input), "load_vocabulary() rejects invalid magic bytes", s_pass, s_fail);

	// Rejection of truncated vocab file
	const std::filesystem::path trunc_vocab_file = "test_trunc.vocab";
	{
		std::ofstream trunc_out(trunc_vocab_file, std::ios::binary);
		trunc_out.write("AX", 2); // only 2 bytes instead of 4+8
	}
	Input trunc_input;
	test_check(!parser.load_vocabulary(trunc_vocab_file, trunc_input), "load_vocabulary() rejects truncated file header", s_pass, s_fail);

	// Rejection of non-existent file
	Input nonexist_input;
	test_check(!parser.load_vocabulary("non_existent_vocab_file.axiom", nonexist_input), "load_vocabulary() fails cleanly on missing file", s_pass, s_fail);

	// Empty text dictionary preserves pre-existing file on disk
	uintmax_t orig_vocab_size = std::filesystem::file_size(test_vocab_file, ec);
	Input empty_in;
	test_check(parser.save_vocabulary(test_vocab_file, empty_in), "save_vocabulary() with empty input returns true without error", s_pass, s_fail);
	test_check_eq(std::filesystem::file_size(test_vocab_file, ec), orig_vocab_size, "save_vocabulary() preserves existing file size on empty dictionary", s_pass, s_fail);

	// 0-token vocabulary file preserves initialized in-memory dictionary
	const std::filesystem::path zero_vocab_file = "test_zero.vocab";
	{
		std::ofstream zero_out(zero_vocab_file, std::ios::binary);
		zero_out.write("AXVK", 4);
		uint64_t zero_sz = 0;
		zero_out.write(reinterpret_cast<const char*>(&zero_sz), sizeof(uint64_t));
	}
	Input preloaded_in;
	preloaded_in.text_dictionary["preserved_token"] = 101;
	preloaded_in.reverse_text_dictionary[101] = "preserved_token";
	test_check(parser.load_vocabulary(zero_vocab_file, preloaded_in), "load_vocabulary() succeeds on 0-token file", s_pass, s_fail);
	test_check_eq(preloaded_in.text_dictionary.size(), static_cast<size_t>(1), "load_vocabulary() preserves in-memory dictionary on 0-token file", s_pass, s_fail);

	// Clean up test files
	std::filesystem::remove(test_vocab_file, ec);
	std::filesystem::remove(corrupt_vocab_file, ec);
	std::filesystem::remove(trunc_vocab_file, ec);
	std::filesystem::remove(zero_vocab_file, ec);

	std::cout << "=== Tokenization & Modality Test Suite: " << s_pass << " passed, " << s_fail << " failed ===\n";
	return s_fail == 0;
}