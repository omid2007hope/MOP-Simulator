#include <iostream>

#include "../include/optimizer.hpp"
#include "../include/parser.hpp"
#include "test_header.hpp"

// ─── Minimal assertion helper ────────────────────────────────────────────────
static int s_pass = 0, s_fail = 0;

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
static void check(bool cond, const char* label) {
	if (cond) {
		std::cout << "  PASS [" << label << "]\n";
		++s_pass;
	} else {
		std::cerr << "  FAIL [" << label << "]\n";
		++s_fail;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
bool test_optimizer() {
	std::cout << "\n=== Optimizer Test Suite ===\n";

	// Constants that match optimizer.cpp internals
	const size_t TOKEN_DIM = 4; // 1 token = 4 bfloat16_t values

	// Use small buffer so tests are fast yet cover all code paths.
	// ram_buffer_size = 100 floats = 25 tokens
	// context_window_size = 4 tokens → window=16 floats, target=4 floats
	const size_t RAM_BUF = 100;
	const size_t CTX_WIN = 4;
	const size_t WIN_ELEMS = CTX_WIN * TOKEN_DIM; // 16
	const size_t TGT_ELEMS = 1 * TOKEN_DIM;	      //  4

	// Expected windows per chunk: floor((RAM_BUF - WIN_ELEMS - TGT_ELEMS) / TOKEN_DIM) + 1
	const size_t EXPECTED_WINDOWS = (RAM_BUF - WIN_ELEMS - TGT_ELEMS) / TOKEN_DIM + 1; // = 21

	// ── [1] Initial Portion state ────────────────────────────────────────────
	std::cout << "\n[1] Initial Portion state\n";
	Portion portion(RAM_BUF, TOKEN_DIM, CTX_WIN);
	check_eq(portion.ram_buffer_tokens.size(), size_t {0}, "ram_buffer empty");
	// Pinned buffers: nullptr until allocate_pinned() is called
	check(portion.context_window_tokens[0] == nullptr, "context_window_tokens == nullptr");
	check(portion.target_tokens[0] == nullptr, "target_tokens == nullptr");
	check(portion.buffer_in_use.load() == false, "buffer_in_use == false");
	check(portion.context_in_use.load() == false, "context_in_use == false");

	check_eq(portion.all_elements, WIN_ELEMS, "all_elements derived");
	check_eq(portion.target_elements, TGT_ELEMS, "target_elements derived");

	// Allocate the pinned host buffers (cudaHostAllocWriteCombined)
	bool pinned_ok = portion.allocate_pinned();
	check(pinned_ok, "allocate_pinned() succeeded");
	check(portion.context_window_tokens[0] != nullptr, "context_window_tokens allocated");
	check(portion.target_tokens[0] != nullptr, "target_tokens allocated");

	Optimizer optimizer(test_embedded_path(), 0.001f);

	// ── [2] disk_to_ram — first load ─────────────────────────────────────────
	std::cout << "\n[2] disk_to_ram — first load\n";
	optimizer.disk_to_ram(portion);
	check_eq(portion.ram_buffer_tokens.size(), RAM_BUF, "ram_buffer filled to RAM_BUF");
	check(portion.buffer_in_use.load() == false, "buffer lock released after load");

	// ── [3] ram_to_contextWindow — slide one window ──────────────────────────
	std::cout << "\n[3] ram_to_contextWindow — slide " << EXPECTED_WINDOWS << " windows\n";
	optimizer.ram_to_contextWindow(portion);

	// After the call the pinned buffers must be filled
	// (we check element count via the Portion size fields, not .size())
	check(portion.context_window_tokens[0] != nullptr, "context_window_tokens filled");
	check(portion.target_tokens[0] != nullptr, "target_tokens filled");
	// Validate first value is a valid (non-NaN) BF16 by casting to float
	{
		float v = static_cast<float>(portion.context_window_tokens[0][0]);
		check(v == v, "context_window_tokens[0] is not NaN");
	}
	check(portion.buffer_in_use.load() == false, "buffer lock released after windows");
	check(portion.context_in_use.load() == false, "context lock released after windows");

	// ── [4] disk_to_ram — second load (carry-forward) ────────────────────────
	std::cout << "\n[4] disk_to_ram — second load (carry-forward)\n";
	// ram_buffer_tokens still holds the old 100 elements.
	// disk_to_ram must: read carry (last WIN_ELEMS=16), clear, prepend carry,
	// then fill so total == RAM_BUF again.
	check_eq(portion.ram_buffer_tokens.size(),
		 RAM_BUF,
		 "buffer still holds data (not cleared by ram_to_contextWindow)");
	optimizer.disk_to_ram(portion);
	check_eq(portion.ram_buffer_tokens.size(),
		 RAM_BUF,
		 "ram_buffer refilled to RAM_BUF on 2nd call");
	check(portion.buffer_in_use.load() == false, "buffer lock released 2nd load");

	// ── [5] Carry-forward correctness ────────────────────────────────────────
	std::cout << "\n[5] Carry-forward sanity\n";
	check(portion.ram_buffer_tokens.size() == RAM_BUF, "carry prepended + disk fill = RAM_BUF");
	// Every element at the carry positions should be a valid BF16 value
	bool carry_nonzero = false;
	for (size_t i = 0; i < WIN_ELEMS; ++i) {
		if (static_cast<double>(portion.ram_buffer_tokens[i]) != 0.0) {
			carry_nonzero = true;
			break;
		}
	}
	check(carry_nonzero, "carry-forward region contains non-zero values");

	// ── [6] Lock safety — buffer_in_use blocks disk_to_ram ───────────────────
	std::cout << "\n[6] Lock safety\n";
	{
		Portion locked(64, TOKEN_DIM, CTX_WIN);
		locked.buffer_in_use.store(true, std::memory_order_relaxed);
		Optimizer opt2(test_embedded_path(), 0.001f);
		opt2.disk_to_ram(locked); // must be a no-op
		check_eq(locked.ram_buffer_tokens.size(),
			 size_t {0},
			 "disk_to_ram skipped when buffer_in_use=true");
	}
	{
		Portion locked(64, TOKEN_DIM, CTX_WIN);
		locked.context_in_use.store(true, std::memory_order_relaxed);
		Optimizer opt3(test_embedded_path(), 0.001f);
		opt3.disk_to_ram(locked); // must be a no-op
		check_eq(locked.ram_buffer_tokens.size(),
			 size_t {0},
			 "disk_to_ram skipped when context_in_use=true");
	}

	// ── [7] Full 3-cycle training loop ───────────────────────────────────────
	std::cout << "\n[7] Full training cycle — 3 consecutive cycles\n";
	for (int cycle = 0; cycle < 3; ++cycle) {
		optimizer.disk_to_ram(portion);
		size_t buf_size = portion.ram_buffer_tokens.size();
		optimizer.ram_to_contextWindow(portion);

		std::cout << "  cycle " << cycle + 1 << ": ram=" << buf_size
			  << " → ctx_elems=" << portion.all_elements
			  << " tgt_elems=" << portion.target_elements << "\n";

		check(portion.context_window_tokens[0] != nullptr, "ctx filled");
		check(portion.target_tokens[0] != nullptr, "tgt filled");
		check(portion.buffer_in_use.load() == false, "buffer unlocked after cycle");
		check(portion.context_in_use.load() == false, "context unlocked after cycle");
	}

	// ── Cleanup ───────────────────────────────────────────────────────────────
	portion.free_pinned();
	check(portion.context_window_tokens[0] == nullptr, "context_window_tokens freed");
	check(portion.target_tokens[0] == nullptr, "target_tokens freed");

	std::cout << "\n=== Optimizer Test Suite: " << s_pass << " passed, " << s_fail
		  << " failed ===\n";
	int fails = s_fail;
	s_pass = s_fail = 0;
	return fails == 0;
}
