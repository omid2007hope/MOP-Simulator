#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../src/CUDA/util/telemetry.cuh"

struct HardwareSpecs {
	std::string gpu_name = "NVIDIA CUDA Device";
	int sm_major = 8;
	int sm_minor = 9;
	int bus_width_bits = 256;
	int mem_clock_khz = 0;
	double peak_bandwidth_gb_s = 716.8; // Default Ada sm_89 (RTX 4080)
	double peak_dense_tflops = 48.74;   // Default Ada sm_89 (RTX 4080)
	size_t free_vram_bytes = 0;
	size_t total_vram_bytes = 0;
};

class AxiomTelemetry {
private:
	void* start_event = nullptr;
	void* stop_event = nullptr;

	std::chrono::high_resolution_clock::time_point cpu_start;
	std::chrono::high_resolution_clock::time_point first_token_time;

	int token_count = 0;
	bool first_token_recorded = false;
	float total_gpu_ms = 0.0f;

	HardwareSpecs specs;

public:
	AxiomTelemetry() {
		start_event = create_cuda_event();
		stop_event = create_cuda_event();
		query_hardware();
	}

	~AxiomTelemetry() {
		if (start_event) {
			destroy_cuda_event(start_event);
			start_event = nullptr;
		}
		if (stop_event) {
			destroy_cuda_event(stop_event);
			stop_event = nullptr;
		}
	}

	void query_hardware() {
		char name_buf[256] = {0};
		int major = 0, minor = 0, bus = 0, clock = 0;
		double bw = 0.0, tflops = 0.0;

		get_hardware_specs(name_buf, sizeof(name_buf),
				   &major, &minor, &bus, &clock,
				   &bw, &tflops);

		if (name_buf[0] != '\0') {
			specs.gpu_name = std::string(name_buf);
		}
		if (major > 0) specs.sm_major = major;
		if (minor >= 0) specs.sm_minor = minor;
		if (bus > 0) specs.bus_width_bits = bus;
		if (clock > 0) specs.mem_clock_khz = clock;
		if (bw > 0.0) specs.peak_bandwidth_gb_s = bw;
		if (tflops > 0.0) specs.peak_dense_tflops = tflops;

		get_gpu_telemetry(&specs.free_vram_bytes, &specs.total_vram_bytes);
	}

	const HardwareSpecs& get_specs() const {
		return specs;
	}

	// Call the exact nanosecond the user hits Enter or prompt inference starts
	void begin_inference() {
		token_count = 0;
		first_token_recorded = false;
		total_gpu_ms = 0.0f;
		cpu_start = std::chrono::high_resolution_clock::now();
		if (start_event) {
			record_cuda_event(start_event, nullptr);
		}
	}

	// Call the exact nanosecond the FIRST token is decoded
	void record_first_token() {
		if (!first_token_recorded) {
			first_token_time = std::chrono::high_resolution_clock::now();
			first_token_recorded = true;
		}
		token_count++;
	}

	// Call for every subsequent token
	void record_token() {
		token_count++;
	}

	// Call when generation is completed
	void end_inference(size_t total_model_parameters,
			   const std::string& title = "BARE-METAL HARDWARE SATURATION") {
		if (stop_event) {
			record_cuda_event(stop_event, nullptr);
			sync_cuda_event(stop_event); // STRICT BARRIER: Wait for hardware execution to physically finish
			total_gpu_ms = elapsed_cuda_events_ms(start_event, stop_event);
		}

		auto cpu_end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> total_cpu_ms = cpu_end - cpu_start;

		double ttft_ms = 0.0;
		if (first_token_recorded) {
			std::chrono::duration<double, std::milli> ttft_dur = first_token_time - cpu_start;
			ttft_ms = ttft_dur.count();
		} else {
			ttft_ms = total_cpu_ms.count();
		}

		if (total_gpu_ms <= 0.0f) {
			total_gpu_ms = static_cast<float>(total_cpu_ms.count());
		}

		// Calculations
		float tok_per_sec = (token_count > 0 && total_gpu_ms > 0.0f)
					? (static_cast<float>(token_count) / total_gpu_ms) * 1000.0f
					: 0.0f;

		// Memory Bandwidth = (parameters * 2 bytes (BF16) * token_count) / time
		double bytes_moved = static_cast<double>(total_model_parameters) * 2.0 * std::max(1, token_count);
		double bandwidth_gb_s = (total_gpu_ms > 0.0f)
					? (bytes_moved / 1e9) / (total_gpu_ms / 1000.0f)
					: 0.0;
		float bandwidth_pct = (specs.peak_bandwidth_gb_s > 0.0)
					? static_cast<float>((bandwidth_gb_s / specs.peak_bandwidth_gb_s) * 100.0)
					: 0.0f;

		// FLOPs = 2 * parameters * tokens (1 multiply, 1 add per weight)
		double flops = 2.0 * static_cast<double>(total_model_parameters) * std::max(1, token_count);
		double achieved_tflops = (total_gpu_ms > 0.0f)
					? (flops / 1e12) / (total_gpu_ms / 1000.0f)
					: 0.0;
		float mfu_pct = (specs.peak_dense_tflops > 0.0)
					? static_cast<float>((achieved_tflops / specs.peak_dense_tflops) * 100.0)
					: 0.0f;

		std::string arch_tag = "sm_" + std::to_string(specs.sm_major) + std::to_string(specs.sm_minor);

		print_compact_hud(ttft_ms, tok_per_sec, bandwidth_gb_s, bandwidth_pct, mfu_pct, arch_tag, title);
	}

	static void print_compact_hud(double ttft_ms, float tok_per_sec,
				       double bandwidth_gb_s, float bandwidth_pct,
				       float mfu_pct, const std::string& arch_tag = "sm_89",
				       const std::string& title = "BARE-METAL HARDWARE SATURATION") {
		float ms_per_tok = (tok_per_sec > 0.0f) ? (1000.0f / tok_per_sec) : 0.0f;

		std::string header = "  PROJECT AXIOM v1.0 • " + title + " [" + arch_tag + "]";
		int visible_len = static_cast<int>(header.length()) - 2; // '•' is 3 bytes in UTF-8 but 1 display column
		int pad = 72 - visible_len;
		if (pad < 0) pad = 0;

		std::cout << "\n"
			  << "┌────────────────────────────────────────────────────────────────────────┐\n"
			  << "│" << header << std::string(pad, ' ') << "│\n"
			  << "├────────────────────────────────────────────────────────────────────────┤\n"
			  << "│  1. PREFILL LATENCY (TTFT)  : " << std::right << std::setw(7) << std::fixed
			  << std::setprecision(1) << ttft_ms
			  << " ms                                │\n"
			  << "│  2. DECODE THROUGHPUT       : " << std::setw(7) << std::setprecision(1)
			  << tok_per_sec << " tok/s  (" << std::setw(5) << std::setprecision(2)
			  << ms_per_tok << " ms/tok)             │\n";

		std::cout << "│  3. MEMORY BUS SATURATION   : ";
		if (bandwidth_gb_s >= 0.1) {
			std::cout << std::setw(6) << std::setprecision(1) << bandwidth_gb_s << " GB/s   ("
				  << std::setw(4) << std::setprecision(1) << bandwidth_pct << "% of Peak)          │\n";
		} else if (bandwidth_gb_s > 0.0001) {
			std::cout << std::setw(6) << std::setprecision(2) << bandwidth_gb_s << " GB/s   (<0.1% of Peak)           │\n";
		} else {
			std::cout << "   0.0 GB/s   ( 0.0% of Peak)          │\n";
		}

		std::cout << "│  4. COMPUTE EFFICIENCY (MFU): ";
		if (mfu_pct >= 0.1f) {
			std::cout << std::setw(5) << std::setprecision(1) << mfu_pct
				  << "% MFU   (Tensor Core Saturation)   │\n";
		} else if (mfu_pct > 0.0001f) {
			std::cout << "<0.1% MFU   (Tensor Core L1-Resident)  │\n";
		} else {
			std::cout << "  0.0% MFU   (Tensor Core L1-Resident)  │\n";
		}

		std::cout << "└────────────────────────────────────────────────────────────────────────┘\n\n";
	}

	static void print_saturation_hud(double sweep_time_ms, double achieved_tflops,
					  double bandwidth_gb_s, float bandwidth_pct,
					  float mfu_pct, const std::string& arch_tag = "sm_89") {
		std::string header = "  PROJECT AXIOM v1.0 • TRANSFORMER KERNEL SATURATION [" + arch_tag + "]";
		int visible_len = static_cast<int>(header.length()) - 2; // '•' is 3 bytes in UTF-8 but 1 display column
		int pad = 72 - visible_len;
		if (pad < 0) pad = 0;

		std::cout << "\n"
			  << "┌────────────────────────────────────────────────────────────────────────┐\n"
			  << "│" << header << std::string(pad, ' ') << "│\n"
			  << "├────────────────────────────────────────────────────────────────────────┤\n"
			  << "│  1. FUSED EVT SWEEP TIME    : " << std::right << std::setw(7) << std::fixed
			  << std::setprecision(2) << sweep_time_ms
			  << " ms                                │\n"
			  << "│  2. SUSTAINED COMPUTE POWER : " << std::setw(7) << std::setprecision(1)
			  << achieved_tflops << " TFLOPs (BF16 Dense)                 │\n";

		std::cout << "│  3. MEMORY BUS SATURATION   : ";
		if (bandwidth_gb_s >= 0.1) {
			std::cout << std::setw(6) << std::setprecision(1) << bandwidth_gb_s << " GB/s   ("
				  << std::setw(4) << std::setprecision(1) << bandwidth_pct << "% of Peak)          │\n";
		} else {
			std::cout << "   0.0 GB/s   ( 0.0% of Peak)          │\n";
		}

		std::cout << "│  4. COMPUTE EFFICIENCY (MFU): ";
		if (mfu_pct >= 0.1f) {
			std::cout << std::setw(5) << std::setprecision(1) << mfu_pct
				  << "% MFU   (Tensor Core Saturation)   │\n";
		} else {
			std::cout << "  0.0% MFU   (Tensor Core Saturation)   │\n";
		}

		std::cout << "└────────────────────────────────────────────────────────────────────────┘\n\n";
	}
};
