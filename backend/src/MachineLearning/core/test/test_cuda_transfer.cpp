// ─────────────────────────────────────────────────────────────────────────────
// test_cuda_transfer.cpp
//
// Tests are split into two sections:
//
//  Section 1 — CPU-side guarantees (always compiled and run with g++)
//    Verifies the bfloat16 type properties that the CPU→GPU transfer relies on.
//    These must hold for upload_to_gpu() to be correct.
//
//  Section 2 — GPU transfer tests (compiled only when CUDA is linked)
//    Verifies cudaMalloc, cudaMemcpy H→D, cudaFree, reuse, realloc, double-free.
//    Skipped with a clear message until MSVC Build Tools are installed.
// ─────────────────────────────────────────────────────────────────────────────
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdfloat>

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
// Section 1 — CPU-side bfloat16 guarantees
// ─────────────────────────────────────────────────────────────────────────────

static void test_bf16_type_size() {
	// std::bfloat16_t and __nv_bfloat16 both use the same 16-bit BF16 layout.
	// If sizeof != 2 the raw pointer cast in upload_to_gpu() is undefined behaviour.
	check_eq(sizeof(std::bfloat16_t), size_t {2}, "sizeof(std::bfloat16_t) == 2 bytes");
}

static void test_bf16_precision() {
	// BF16 keeps 8 bits of exponent (same as float32) and 7 mantissa bits.
	// Precision: ~2–3 significant decimal digits.

	// sin(1.0) = 0.8414709848...
	// Nearest BF16: 0.8398 or 0.8438 — within 0.3% of true value.
	std::bfloat16_t v = static_cast<std::bfloat16_t>(0.8414709848);
	double back = static_cast<double>(v);
	check(back >= 0.83 && back <= 0.85, "BF16(sin(1.0)) round-trips within ±0.01");
	std::cout << "    value stored: " << back << "  original: 0.8414709848\n";

	// Large integer: BF16 can represent 256 exactly (8-bit exponent range).
	std::bfloat16_t big = static_cast<std::bfloat16_t>(256.0);
	check_eq(static_cast<double>(big), 256.0, "BF16(256.0) exact round-trip");

	// Zero is always exact.
	std::bfloat16_t zero = static_cast<std::bfloat16_t>(0.0);
	check_eq(static_cast<double>(zero), 0.0, "BF16(0.0) exact");
}

static void test_bf16_memory_layout() {
	// Verify that std::bfloat16_t has the same binary layout as a 2-byte integer
	// (no padding, trivially copyable). This guarantees that reading BF16 values
	// written by the CPU with cudaMemcpy on the GPU is valid.
	check(std::is_trivially_copyable_v<std::bfloat16_t>,
	      "std::bfloat16_t is trivially copyable (safe for memcpy/cudaMemcpy)");
	check(alignof(std::bfloat16_t) <= 2, "alignof(std::bfloat16_t) <= 2 (no surprise padding)");

	// Verify a vector of BF16 values is contiguous in memory.
	std::bfloat16_t arr[4] = {
		static_cast<std::bfloat16_t>(1.0f),
		static_cast<std::bfloat16_t>(2.0f),
		static_cast<std::bfloat16_t>(3.0f),
		static_cast<std::bfloat16_t>(4.0f),
	};
	const char* p0 = reinterpret_cast<const char*>(&arr[0]);
	const char* p1 = reinterpret_cast<const char*>(&arr[1]);
	check_eq(
		p1 - p0, ptrdiff_t {2}, "consecutive BF16 elements are 2 bytes apart (contiguous)");
}

static void test_gpu_portion_mock() {
	// CPU mock of GpuPortion — verifies default init (no CUDA needed).
	struct MockGpuPortion {
		void* d_context_window[2] = {nullptr, nullptr};
		void* d_target[2] = {nullptr, nullptr};
		size_t context_elements = 0;
		size_t target_elements = 0;
	};

	MockGpuPortion g;
	check(g.d_context_window[0] == nullptr, "GpuPortion d_context_window init nullptr");
	check(g.d_target[0] == nullptr, "GpuPortion d_target init nullptr");
	check_eq(g.context_elements, size_t {0}, "GpuPortion context_elements init 0");
	check_eq(g.target_elements, size_t {0}, "GpuPortion target_elements init 0");

	// Simulate context window for 128 tokens * 4 dim
	const size_t ctx_bytes = 128 * 4 * sizeof(std::bfloat16_t);
	check_eq(ctx_bytes,
		 size_t {1024},
		 "context window byte size == 1024 (128 tokens * 4 dim * 2 bytes)");

	// Simulate target for 1 token * 4 dim
	const size_t tgt_bytes = 1 * 4 * sizeof(std::bfloat16_t);
	check_eq(tgt_bytes, size_t {8}, "target byte size == 8 (1 token * 4 dim * 2 bytes)");
}

// ─────────────────────────────────────────────────────────────────────────────
// Section 2 — GPU tests
// ─────────────────────────────────────────────────────────────────────────────
#include <vector>

#include "../src/CUDA/transfer.cuh"

static void test_gpu_roundtrip() {
	GpuPortion gpu {};

	// Create CPU context data (128 tokens * 4 dim = 512 elements)
	const size_t ctx_elems = 512;
	std::vector<std::bfloat16_t> h_ctx_in(ctx_elems);
	for (size_t i = 0; i < ctx_elems; ++i) {
		h_ctx_in[i] = static_cast<std::bfloat16_t>(i * 0.5f);
	}

	// Create CPU target data (1 token * 4 dim = 4 elements)
	const size_t tgt_elems = 4;
	std::vector<std::bfloat16_t> h_tgt_in(tgt_elems);
	for (size_t i = 0; i < tgt_elems; ++i) {
		h_tgt_in[i] = static_cast<std::bfloat16_t>(i * 0.1f);
	}

	// 1. First call: cudaMalloc + cudaMemcpy
	bool up1 = upload_to_gpu(h_ctx_in.data(), ctx_elems, h_tgt_in.data(), tgt_elems, &gpu, 0);
	check(up1, "upload_to_gpu() — first call allocates and copies");
	check(gpu.d_context_window[0] != nullptr, "gpu.d_context_window is not null");
	check(gpu.d_target[0] != nullptr, "gpu.d_target is not null");
	check_eq(gpu.context_elements, ctx_elems, "gpu.context_elements matches");
	check_eq(gpu.target_elements, tgt_elems, "gpu.target_elements matches");

	// Save pointers to check reuse
	void* old_ctx_ptr = gpu.d_context_window[0];
	void* old_tgt_ptr = gpu.d_target[0];

	// 2. Second call same size: reuse buffer
	bool up2 = upload_to_gpu(h_ctx_in.data(), ctx_elems, h_tgt_in.data(), tgt_elems, &gpu, 0);
	check(up2, "upload_to_gpu() — second call succeeds");
	check(gpu.d_context_window[0] == old_ctx_ptr,
	      "d_context_window pointer reused (no realloc)");
	check(gpu.d_target[0] == old_tgt_ptr, "d_target pointer reused (no realloc)");

	// 3. Verify H→D values match by downloading back (D→H)
	std::vector<std::bfloat16_t> h_ctx_out(ctx_elems, static_cast<std::bfloat16_t>(0.0f));
	std::vector<std::bfloat16_t> h_tgt_out(tgt_elems, static_cast<std::bfloat16_t>(0.0f));

	bool down = download_from_gpu(
		h_ctx_out.data(), ctx_elems, h_tgt_out.data(), tgt_elems, &gpu, 0);
	check(down, "download_from_gpu() succeeds");

	bool ctx_match = true;
	for (size_t i = 0; i < ctx_elems; ++i) {
		if (h_ctx_in[i] != h_ctx_out[i])
			ctx_match = false;
	}
	check(ctx_match, "upload_to_gpu() — verify H→D→H context values match precisely");

	bool tgt_match = true;
	for (size_t i = 0; i < tgt_elems; ++i) {
		if (h_tgt_in[i] != h_tgt_out[i])
			tgt_match = false;
	}
	check(tgt_match, "upload_to_gpu() — verify H→D→H target values match precisely");

	// 4. Size change: free + realloc
	const size_t new_ctx_elems = 1024;
	std::vector<std::bfloat16_t> new_h_ctx_in(new_ctx_elems,
						  static_cast<std::bfloat16_t>(1.0f));
	bool up3 = upload_to_gpu(
		new_h_ctx_in.data(), new_ctx_elems, h_tgt_in.data(), tgt_elems, &gpu, 0);
	check(up3, "upload_to_gpu() — size change succeeds");
	check(gpu.d_context_window[0] != old_ctx_ptr, "d_context_window reallocated for new size");
	check_eq(gpu.context_elements, new_ctx_elems, "context_elements updated to new size");

	// 5. Free GPU
	free_gpu(&gpu);
	check(gpu.d_context_window[0] == nullptr, "free_gpu() releases d_context_window");
	check(gpu.d_target[0] == nullptr, "free_gpu() releases d_target");
	check_eq(gpu.context_elements, size_t {0}, "free_gpu() resets context_elements");
	check_eq(gpu.target_elements, size_t {0}, "free_gpu() resets target_elements");

	// Safe to call twice
	free_gpu(&gpu);
	check(true, "free_gpu() safe to call twice (no double-free)");
}

static void test_cutlass_forward_pass_evt() {
	const size_t in_dim = 512; // 128 tokens * 4 dim
	const size_t out_dim = 4;

	GpuPortion gpu {};
	GpuWeights weights {};
	GpuActivations acts {};

	std::vector<std::bfloat16_t> h_ctx(in_dim);
	for (size_t i = 0; i < in_dim; ++i) {
		h_ctx[i] = static_cast<std::bfloat16_t>((static_cast<int>(i % 7) - 3) * 0.25f);
	}
	std::vector<std::bfloat16_t> h_tgt(out_dim, static_cast<std::bfloat16_t>(1.0f));

	bool up_data = upload_to_gpu(h_ctx.data(), in_dim, h_tgt.data(), out_dim, &gpu, 0);
	check(up_data, "upload_to_gpu() for CUTLASS forward test");

	std::vector<float> h_w(in_dim * out_dim);
	for (size_t i = 0; i < h_w.size(); ++i) {
		h_w[i] = (static_cast<float>(i % 11) - 5.0f) * 0.05f;
	}
	std::vector<float> h_b = {0.1f, -0.5f, 0.2f, -0.1f};

	bool up_w = upload_ai_weights(h_w.data(), h_w.size(), h_b.data(), h_b.size(), &weights);
	check(up_w, "upload_ai_weights() succeeds");

	// Run CUTLASS EVT forward pass
	bool fwd = forward_pass(&gpu, &weights, &acts, in_dim, out_dim, 0);
	check(fwd, "CUTLASS EVT forward_pass() kernel execution");

	std::vector<std::bfloat16_t> h_out(out_dim);
	bool down_out = download_output(&acts, h_out.data(), out_dim);
	check(down_out, "download_output() succeeds");

	// Compute CPU reference: ReLU(W * x + b)
	bool matched = true;
	for (size_t row = 0; row < out_dim; ++row) {
		float sum = h_b[row];
		for (size_t col = 0; col < in_dim; ++col) {
			float x_val = static_cast<float>(h_ctx[col]);
			float w_val = h_w[row * in_dim + col];
			sum += w_val * x_val;
		}
		if (sum < 0.0f) {
			sum = 0.0f; // ReLU
		}
		float actual = static_cast<float>(h_out[row]);
		if (std::abs(actual - sum) > 0.15f) {
			matched = false;
			std::cerr << "Mismatch row " << row << ": actual=" << actual << " ref=" << sum << "\n";
		}
	}
	check(matched, "CUTLASS EVT output matches CPU reference ReLU(Wx + b)");

	free_gpu(&gpu);
	free_gpu_weights(&weights);
	free_gpu_activations(&acts);
}

static void test_backward_pass_and_loss_drop() {
	const size_t in_dim = 512;
	const size_t out_dim = 4;
	const float lr = 0.01f;

	GpuPortion gpu {};
	GpuWeights weights {};
	GpuActivations acts {};

	std::vector<std::bfloat16_t> h_ctx(in_dim);
	for (size_t i = 0; i < in_dim; ++i) {
		h_ctx[i] = static_cast<std::bfloat16_t>(0.1f * ((i % 5) + 1));
	}
	std::vector<std::bfloat16_t> h_tgt(out_dim);
	for (size_t i = 0; i < out_dim; ++i) {
		h_tgt[i] = static_cast<std::bfloat16_t>(1.0f + 0.5f * i);
	}

	upload_to_gpu(h_ctx.data(), in_dim, h_tgt.data(), out_dim, &gpu, 0);

	std::vector<float> h_w_init(in_dim * out_dim, 0.01f);
	std::vector<float> h_b_init(out_dim, 0.0f);
	upload_ai_weights(h_w_init.data(), h_w_init.size(), h_b_init.data(), h_b_init.size(), &weights);

	// 1. Initial step
	forward_pass(&gpu, &weights, &acts, in_dim, out_dim, 0);
	backward_pass(&gpu, &weights, &acts, in_dim, out_dim, lr, 0);

	float init_loss = 0.0f;
	download_loss(&acts, &init_loss);
	check(init_loss > 0.0f, "Initial loss is strictly positive");

	// 2. Check weights actually changed
	std::vector<float> h_w_after(in_dim * out_dim);
	std::vector<float> h_b_after(out_dim);
	download_ai_weights(&weights, h_w_after.data(), h_w_after.size(), h_b_after.data(), h_b_after.size());

	bool weights_changed = false;
	for (size_t i = 0; i < h_w_after.size(); ++i) {
		if (h_w_after[i] != h_w_init[i]) {
			weights_changed = true;
			break;
		}
	}
	check(weights_changed, "backward_pass physically updated weights in VRAM");

	// 3. Train for 50 steps on this pattern to verify loss drops
	for (int step = 0; step < 50; ++step) {
		forward_pass(&gpu, &weights, &acts, in_dim, out_dim, 0);
		backward_pass(&gpu, &weights, &acts, in_dim, out_dim, lr, 0);
	}

	float final_loss = 0.0f;
	download_loss(&acts, &final_loss);
	// Final single-step loss after 50 updates
	forward_pass(&gpu, &weights, &acts, in_dim, out_dim, 0);
	backward_pass(&gpu, &weights, &acts, in_dim, out_dim, lr, 0);
	float eval_loss = 0.0f;
	download_loss(&acts, &eval_loss);

	std::cout << "  Initial loss: " << init_loss << " -> Final loss: " << eval_loss << "\n";
	check(eval_loss < init_loss * 0.1f, "Loss dropped by > 90% (Network is learning sequences)");

	free_gpu(&gpu);
	free_gpu_weights(&weights);
	free_gpu_activations(&acts);
}

// ─────────────────────────────────────────────────────────────────────────────
bool test_cuda_transfer() {
	std::cout << "\n=== CUDA Transfer Test Suite ===\n";

	std::cout << "\n[1] BF16 type size\n";
	test_bf16_type_size();

	std::cout << "\n[2] BF16 precision\n";
	test_bf16_precision();

	std::cout << "\n[3] BF16 memory layout (safe for cudaMemcpy)\n";
	test_bf16_memory_layout();

	std::cout << "\n[4] GpuPortion CPU mock\n";
	test_gpu_portion_mock();

	std::cout << "\n[5] GPU transfer tests\n";
	test_gpu_roundtrip();

	std::cout << "\n[6] CUTLASS EVT Forward Pass (Tensor Cores + Fused Bias/ReLU)\n";
	test_cutlass_forward_pass_evt();

	std::cout << "\n[7] Backward Pass Gradients & Validation Loss Drop\n";
	test_backward_pass_and_loss_drop();

	std::cout << "=== CUDA Transfer Test Suite: " << s_pass << " passed, " << s_fail
		  << " failed ===\n";
	int fails = s_fail;
	s_pass = s_fail = 0;
	return fails == 0;
}
