#ifndef include_optimizer
#define include_optimizer

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "parser.hpp"

// ── DLL-imported pinned memory API ──────────────────────────────────────────
// All cudaHostAlloc / cudaFreeHost calls live inside cuda_transfer.dll
// (compiled by nvcc/MSVC). The g++ side only calls these two thin wrappers,
// so it never needs to link against cudart.lib or any CUDA import library.
#ifndef CUDA_API
#define CUDA_API __declspec(dllimport)
#endif
extern "C" CUDA_API void* pinned_alloc(size_t bytes);
extern "C" CUDA_API void pinned_free(void* ptr);

// Architecture principle: Storage drives are hyper-optimized for sequential
// forward reads.

struct Portion {

	int buffer_index = 0;

	// =========================================================================
	// CRITICAL SYNCHRONIZATION BARRIER — LIFE AND DEATH GUARANTEE
	// -------------------------------------------------------------------------
	// PAY MAXIMUM ATTENTION: If you mess up these synchronization flags, you
	// risk corrupting both historical and ongoing training data across the
	// entire pipeline — causing millions of dollars in wasted compute, forcing
	// a full wipe back to zero, and resulting in catastrophic failure.
	// DO NOT tamper with or bypass these state locks under any circumstances!
	// =========================================================================
	// =========================================================================
	// UPGRADED: primitive bool → std::atomic<bool>
	// Atomic flags guarantee that reads and writes across CPU threads are
	// sequentially consistent — no torn reads, no UB on concurrent access.
	// =========================================================================
	std::atomic<bool> buffer_in_use {false};
	std::atomic<bool> context_in_use {false};
	// =========================================================================

	// ---------------------------------------------------------
	// Tier 1: The RAM Buffer (Disk -> RAM)
	// Optimal size: 100,000 to 1,000,000 tokens to keep CPU fed.
	// ---------------------------------------------------------
	size_t ram_buffer_size;

	// Holds: ram_buffer_size * embedding_dim (e.g., 400,000 bfloat16_t values)
	std::vector<std::bfloat16_t> ram_buffer_tokens = {};

	// ---------------------------------------------------------
	// Tier 2: The AI's Field of View (RAM -> AI)
	// Optimal for basic AI: 64 or 128 tokens.
	// ---------------------------------------------------------
	// 1 token = token_dim bfloat16_t values (e.g., 4)
	size_t token_dim;

	size_t context_window_size;

	// context_window_size * token_dim (e.g., 128 * 4 = 512)
	size_t all_elements;

	// 1 * token_dim (e.g., 1 * 4 = 4)
	size_t target_elements;

	size_t offset = 0;

	// All sizes are decided by the caller (main); the derived element counts
	// are computed here so they can never go out of sync with them.
	Portion(size_t ram_buffer_size, size_t token_dim, size_t context_window_size)
	    : ram_buffer_size(ram_buffer_size),
	      token_dim(token_dim),
	      context_window_size(context_window_size),
	      all_elements(context_window_size * token_dim),
	      target_elements(1 * token_dim) {}

	// =========================================================================
	// UPGRADED: pageable std::vector → cudaHostAllocWriteCombined pinned memory
	//
	// cudaMemcpyAsync silently degrades to a synchronous blocking call when
	// handed a pageable heap pointer. Pinning these two buffers with
	// cudaHostAllocWriteCombined achieves three goals:
	//   1. Locks the physical page so the GPU DMA engine can read it directly.
	//   2. Bypasses the CPU L1/L2 cache hierarchy (write-combining), pushing
	//      data straight to the PCIe bus for up to 40% higher throughput.
	//   3. Enables true cudaMemcpyAsync (non-blocking on the host thread).
	// =========================================================================

	// The Input (X): pinned host buffer — 128 tokens * 4 dim = 512 bfloat16_t
	std::bfloat16_t* context_window_tokens[2] = {nullptr, nullptr};

	// The Target (Y): pinned host buffer — 1 token * 4 dim = 4 bfloat16_t
	std::bfloat16_t* target_tokens[2] = {nullptr, nullptr};

	// Allocate the pinned buffers. Must be called once before the training loop.
	// Returns false if the CUDA DLL reports an allocation failure.
	bool allocate_pinned() {
		if (context_window_tokens[0] || target_tokens[0]) {
			return true; // already allocated
		}

		for (int i = 0; i < 2; ++i) {
			context_window_tokens[i] = static_cast<std::bfloat16_t*>(
				pinned_alloc(all_elements * sizeof(std::bfloat16_t)));

			target_tokens[i] = static_cast<std::bfloat16_t*>(
				pinned_alloc(target_elements * sizeof(std::bfloat16_t)));

			if (!context_window_tokens[i] || !target_tokens[i]) {
				std::cerr << "[Portion] pinned_alloc failed\n";
				free_pinned();
				return false;
			}
		}

		return true;
	}

	Portion(const Portion&) = delete;
	Portion& operator=(const Portion&) = delete;

	~Portion() {
		free_pinned();
	}

	// Release pinned host memory. Safe to call even if allocate_pinned() was
	// never called (pinned_free checks for nullptr internally).
	void free_pinned() {
		for (int i = 0; i < 2; ++i) {
			if (context_window_tokens[i]) {
				pinned_free(context_window_tokens[i]);
				context_window_tokens[i] = nullptr;
			}

			if (target_tokens[i]) {
				pinned_free(target_tokens[i]);
				target_tokens[i] = nullptr;
			}
		}
	}
};

class Optimizer {
private:
	// Kept permanently open: ~8KB OS buffer cost, zero syscall overhead per
	// chunk.
	std::ifstream file_stream;

public:
	// Path to the binary embedded data file produced by Parser::embedding().
	// Decided by the caller (main) and read by disk_to_ram().
	std::filesystem::path data_path;

	// Gradient-descent step size used by AI::train(); decided by the caller.
	float learning_rate;

	Optimizer(std::filesystem::path data_path, float learning_rate)
	    : data_path(std::move(data_path)), learning_rate(learning_rate) {}

	~Optimizer() {
		close_stream();
	}

	void close_stream() {
		if (file_stream.is_open()) {
			file_stream.close();
		}
	}

	void disk_to_ram(Portion& portion);

	bool ram_to_contextWindow(Portion& portion);
};

#endif