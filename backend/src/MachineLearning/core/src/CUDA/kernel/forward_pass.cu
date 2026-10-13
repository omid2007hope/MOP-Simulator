#include <cuda_bf16.h>
#include <cuda_runtime.h>

#include "../util/cuda_utils.cuh"
#include "forward_pass.cuh"

// The CUTLASS visitor headers are not self-contained: gemm_universal.h must come
// first, so this order must not be re-sorted.
// clang-format off
#include "cutlass/cutlass.h"
#include "cutlass/gemm/device/gemm_universal.h"
#include "cutlass/epilogue/thread/activation.h"
#include "cutlass/epilogue/threadblock/fusion/visitors.hpp"
#include "cutlass/gemm/kernel/default_gemm_universal_with_visitor.h"
#include "cutlass/gemm/device/gemm_universal_adapter.h"
// clang-format on

// =============================================================================
// Forward pass:  y = ReLU(W x + b)  as ONE fused CUTLASS kernel on the Tensor Cores
//
// The layer is a GEMM   Y[M x N] = X[M x K] * W^T[K x N]
//     M = 1         (one context window per step)
//     N = out_dim   (one output per neuron)
//     K = in_dim    (window elements)
//
// The multiply runs on the Tensor Cores (bf16 inputs, FP32 accumulation). The
// "epilogue" that normally needs separate kernels, each one writing to VRAM and
// reading it back, is an Epilogue Visitor Tree (EVT) executed on the accumulators
// while they are still in registers:
//
//              Store(bf16)
//                  |
//             ReLU(bf16 <- fp32)
//                  |
//          Add( Accum , Bias )
//           /               \
//   VisitorAccFetch    VisitorRowBroadcast
// =============================================================================
namespace {

using ElementIn = cutlass::bfloat16_t; // layout-compatible with __nv_bfloat16
using ElementAccumulator = float;
using ElementCompute = float;

using LayoutX = cutlass::layout::RowMajor;    // x: K contiguous
using LayoutW = cutlass::layout::ColumnMajor; // W is stored [out][in] == W^T column-major
using LayoutY = cutlass::layout::RowMajor;

constexpr int AlignmentIn = 8;	// 128-bit vector loads of bf16 (in_dim must be a multiple of 8)
constexpr int AlignmentOut = 1; // out_dim is tiny, so it is not forced to a multiple of 8

using ThreadblockShape = cutlass::gemm::GemmShape<64, 64, 32>;
using WarpShape = cutlass::gemm::GemmShape<32, 32, 32>;
using InstructionShape = cutlass::gemm::GemmShape<16, 8, 16>; // bf16 mma.sync m16n8k16
constexpr int NumStages = 4;
constexpr int EpilogueStages = 1;

using OutputTileThreadMap = cutlass::epilogue::threadblock::
	OutputTileThreadLayout<ThreadblockShape, WarpShape, ElementIn, AlignmentOut, EpilogueStages>;

using Accum = cutlass::epilogue::threadblock::VisitorAccFetch;

// One bias value per output column, repeated for every row (stride 0 along M)
using Bias = cutlass::epilogue::threadblock::
	VisitorRowBroadcast<OutputTileThreadMap, ElementIn, cute::Stride<cute::_0, cute::_1, int32_t>>;

using AddBias = cutlass::epilogue::threadblock::VisitorCompute<cutlass::plus,
							       ElementCompute,
							       ElementCompute,
							       cutlass::FloatRoundStyle::round_to_nearest>;

using EvtAddBias = cutlass::epilogue::threadblock::Sm80EVT<AddBias, Accum, Bias>;

using Relu = cutlass::epilogue::threadblock::VisitorCompute<cutlass::epilogue::thread::ReLu,
							    ElementIn,
							    ElementCompute,
							    cutlass::FloatRoundStyle::round_to_nearest>;

using EvtRelu = cutlass::epilogue::threadblock::Sm80EVT<Relu, EvtAddBias>;

using Store = cutlass::epilogue::threadblock::VisitorAuxStore<
	OutputTileThreadMap,
	ElementIn,
	cutlass::FloatRoundStyle::round_to_nearest,
	cute::Stride<int64_t, cute::_1, int64_t>>;

using EvtStore = cutlass::epilogue::threadblock::Sm80EVT<Store, EvtRelu>;

using GemmKernel = typename cutlass::gemm::kernel::DefaultGemmWithVisitor<
	ElementIn,
	LayoutX,
	cutlass::ComplexTransform::kNone,
	AlignmentIn,
	ElementIn,
	LayoutW,
	cutlass::ComplexTransform::kNone,
	AlignmentIn,
	ElementIn,
	LayoutY,
	AlignmentOut,
	ElementAccumulator,
	ElementCompute,
	cutlass::arch::OpClassTensorOp,
	cutlass::arch::Sm80, // Ada (sm_89) runs the Ampere Tensor Core kernels
	ThreadblockShape,
	WarpShape,
	InstructionShape,
	EvtStore,
	cutlass::gemm::threadblock::GemmIdentityThreadblockSwizzle<>,
	NumStages,
	cutlass::arch::OpMultiplyAdd,
	EpilogueStages>::GemmKernel;

using Gemm = cutlass::gemm::device::GemmUniversalAdapter<GemmKernel>;

} // namespace

extern "C" CUDA_API bool forward_pass(const GpuPortion* gpu_portion,
				      const GpuWeights* gpu_weights,
				      GpuActivations* gpu_activations,
				      size_t input_dim,
				      size_t output_dim,
				      int buffer_index) {

	// Safe Guard
	if (!gpu_portion || !gpu_weights || !gpu_activations ||
	    !gpu_portion->d_context_window[buffer_index] || !gpu_weights->d_weights ||
	    !gpu_weights->d_bias || input_dim == 0 || output_dim == 0) {
		return false;
	}

	if (gpu_activations->output_elements != output_dim) {
		if (gpu_activations->d_output) {
			if (!CUDA_CHECK(cudaFree(gpu_activations->d_output)))
				return false;
			gpu_activations->d_output = nullptr;
		}

		if (!CUDA_CHECK(cudaMalloc(&gpu_activations->d_output,
					   output_dim * sizeof(__nv_bfloat16))))
			return false;
		gpu_activations->output_elements = output_dim;
	}

	// The loss accumulator is written by the backward pass; it starts at zero
	if (!gpu_activations->d_loss) {
		if (!CUDA_CHECK(cudaMalloc(&gpu_activations->d_loss, sizeof(float))))
			return false;
		if (!CUDA_CHECK(cudaMemset(gpu_activations->d_loss, 0, sizeof(float))))
			return false;
	}

	cudaStream_t stream = (cudaStream_t)gpu_portion->stream[buffer_index];

	// The two buffers share the weights and the output, so this step may only start
	// after the previous step (other buffer) finished updating them. Its upload is
	// already queued on this stream and still overlaps with that step.
	cudaEvent_t previous_step_done = (cudaEvent_t)gpu_portion->compute_done[1 - buffer_index];
	if (previous_step_done) {
		if (!CUDA_CHECK(cudaStreamWaitEvent(stream, previous_step_done, 0)))
			return false;
	}

	using cute::_0;
	using cute::_1;

	const int M = 1;
	const int N = static_cast<int>(output_dim);
	const int K = static_cast<int>(input_dim);

	// Visitor arguments are listed children first, then the node itself
	typename EvtStore::Arguments callback_args {
		{
			{
				{}, // Accum
				{static_cast<const ElementIn*>(gpu_weights->d_bias),
				 ElementIn(0),
				 {_0 {}, _1 {}, int32_t(N)}}, // Bias
				{}			       // AddBias
			},				       // EvtAddBias
			{}				       // Relu
		},					       // EvtRelu
		{static_cast<ElementIn*>(gpu_activations->d_output),
		 {int64_t(N), _1 {}, int64_t(M) * N}} // Store
	};

	typename Gemm::Arguments arguments(
		cutlass::gemm::GemmUniversalMode::kGemm,
		cutlass::gemm::GemmCoord(M, N, K),
		1, // batch_count
		callback_args,
		gpu_portion->d_context_window[buffer_index], // ptr_A: x
		gpu_weights->d_weights,			     // ptr_B: W
		nullptr,				     // ptr_C (the EVT does the store)
		nullptr,				     // ptr_D
		int64_t(0),				     // batch_stride_A
		int64_t(0),				     // batch_stride_B
		int64_t(0),				     // batch_stride_C
		int64_t(0),				     // batch_stride_D
		int64_t(K),				     // lda
		int64_t(K),				     // ldb
		int64_t(0),				     // ldc
		int64_t(0));				     // ldd

	Gemm gemm_op;

	cutlass::Status status = gemm_op.can_implement(arguments);
	if (status != cutlass::Status::kSuccess) {
		fprintf(stderr,
			"[CUTLASS] forward_pass unsupported (in_dim=%zu must be a multiple of %d): %s\n",
			input_dim,
			AlignmentIn,
			cutlass::cutlassGetStatusString(status));
		return false;
	}

	// A single-slice GEMM needs no workspace
	if (Gemm::get_workspace_size(arguments) != 0) {
		fprintf(stderr, "[CUTLASS] forward_pass unexpectedly needs a workspace\n");
		return false;
	}

	status = gemm_op(arguments, nullptr, stream);
	if (status != cutlass::Status::kSuccess) {
		fprintf(stderr,
			"[CUTLASS] forward_pass failed: %s\n",
			cutlass::cutlassGetStatusString(status));
		return false;
	}

	return CUDA_CHECK(cudaGetLastError());
}

extern "C" CUDA_API bool run_cutlass_saturation_benchmark(int M, int N, int K, int iterations,
							 double* out_tflops,
							 double* out_bandwidth_gb_s,
							 float* out_avg_ms) {
	if (M <= 0 || N <= 0 || K <= 0 || iterations <= 0) {
		return false;
	}

	if (K % AlignmentIn != 0) {
		return false;
	}

	using cute::_0;
	using cute::_1;

	size_t size_A = static_cast<size_t>(M) * K * sizeof(__nv_bfloat16);
	size_t size_B = static_cast<size_t>(K) * N * sizeof(__nv_bfloat16);
	size_t size_bias = static_cast<size_t>(N) * sizeof(__nv_bfloat16);
	size_t size_out = static_cast<size_t>(M) * N * sizeof(__nv_bfloat16);

	__nv_bfloat16* d_A = nullptr;
	__nv_bfloat16* d_B = nullptr;
	__nv_bfloat16* d_bias = nullptr;
	__nv_bfloat16* d_out = nullptr;

	if (cudaMalloc(&d_A, size_A) != cudaSuccess ||
	    cudaMalloc(&d_B, size_B) != cudaSuccess ||
	    cudaMalloc(&d_bias, size_bias) != cudaSuccess ||
	    cudaMalloc(&d_out, size_out) != cudaSuccess) {
		if (d_A) cudaFree(d_A);
		if (d_B) cudaFree(d_B);
		if (d_bias) cudaFree(d_bias);
		if (d_out) cudaFree(d_out);
		return false;
	}

	cudaMemset(d_A, 0x3c, size_A);
	cudaMemset(d_B, 0x3c, size_B);
	cudaMemset(d_bias, 0x00, size_bias);
	cudaMemset(d_out, 0x00, size_out);

	typename EvtStore::Arguments callback_args {
		{
			{
				{}, // Accum
				{reinterpret_cast<const ElementIn*>(d_bias),
				 ElementIn(0),
				 {_0 {}, _1 {}, int32_t(N)}}, // Bias
				{}			       // AddBias
			},				       // EvtAddBias
			{}				       // Relu
		},					       // EvtRelu
		{reinterpret_cast<ElementIn*>(d_out),
		 {int64_t(N), _1 {}, int64_t(M) * N}} // Store
	};

	typename Gemm::Arguments arguments(
		cutlass::gemm::GemmUniversalMode::kGemm,
		cutlass::gemm::GemmCoord(M, N, K),
		1, // batch_count
		callback_args,
		reinterpret_cast<ElementIn*>(d_A),
		reinterpret_cast<ElementIn*>(d_B),
		nullptr,
		nullptr,
		int64_t(0),
		int64_t(0),
		int64_t(0),
		int64_t(0),
		int64_t(K),
		int64_t(K),
		int64_t(0),
		int64_t(0));

	Gemm gemm_op;

	cutlass::Status status = gemm_op.can_implement(arguments);
	if (status != cutlass::Status::kSuccess) {
		cudaFree(d_A);
		cudaFree(d_B);
		cudaFree(d_bias);
		cudaFree(d_out);
		return false;
	}

	size_t workspace_size = Gemm::get_workspace_size(arguments);
	void* d_workspace = nullptr;
	if (workspace_size > 0) {
		if (cudaMalloc(&d_workspace, workspace_size) != cudaSuccess) {
			cudaFree(d_A);
			cudaFree(d_B);
			cudaFree(d_bias);
			cudaFree(d_out);
			return false;
		}
	}

	cudaStream_t stream = nullptr;

	// Warm-up passes
	for (int i = 0; i < 5; ++i) {
		gemm_op(arguments, d_workspace, stream);
	}
	cudaStreamSynchronize(stream);

	cudaEvent_t start = nullptr, stop = nullptr;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);

	cudaEventRecord(start, stream);
	for (int i = 0; i < iterations; ++i) {
		gemm_op(arguments, d_workspace, stream);
	}
	cudaEventRecord(stop, stream);
	cudaEventSynchronize(stop);

	float total_ms = 0.0f;
	cudaEventElapsedTime(&total_ms, start, stop);

	cudaEventDestroy(start);
	cudaEventDestroy(stop);

	if (d_workspace) cudaFree(d_workspace);
	cudaFree(d_A);
	cudaFree(d_B);
	cudaFree(d_bias);
	cudaFree(d_out);

	if (total_ms <= 0.0f) {
		return false;
	}

	float avg_ms = total_ms / static_cast<float>(iterations);
	if (out_avg_ms) *out_avg_ms = avg_ms;

	double flops_per_iter = 2.0 * static_cast<double>(M) * static_cast<double>(N) * static_cast<double>(K);
	double total_flops = flops_per_iter * static_cast<double>(iterations);
	double achieved_tflops = (total_flops / 1e12) / (total_ms / 1000.0);
	if (out_tflops) *out_tflops = achieved_tflops;

	double bytes_per_iter = static_cast<double>(size_A + size_B + size_bias + size_out);
	double total_bytes = bytes_per_iter * static_cast<double>(iterations);
	double achieved_bw_gb_s = (total_bytes / 1e9) / (total_ms / 1000.0);
	if (out_bandwidth_gb_s) *out_bandwidth_gb_s = achieved_bw_gb_s;

	return true;
}

