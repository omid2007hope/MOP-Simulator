#include <cstring>
#include <cuda_runtime.h>
#include <iostream>

#include "telemetry.cuh"

extern "C" void get_gpu_telemetry(size_t* free_mem, size_t* total_mem) {
	if (free_mem && total_mem) {
		cudaError_t err = cudaMemGetInfo(free_mem, total_mem);
		if (err != cudaSuccess) {
			*free_mem = 0;
			*total_mem = 0;
		}
	}
}

extern "C" void get_hardware_specs(char* name_buf, int name_buf_len,
                                   int* sm_major, int* sm_minor,
                                   int* bus_width_bits, int* mem_clock_khz,
                                   double* peak_bandwidth_gb_s, double* peak_dense_tflops) {
	cudaDeviceProp prop;
	if (cudaGetDeviceProperties(&prop, 0) == cudaSuccess) {
		if (name_buf && name_buf_len > 0) {
			strncpy(name_buf, prop.name, name_buf_len - 1);
			name_buf[name_buf_len - 1] = '\0';
		}
	}

	int major = 0, minor = 0, bus_width = 0, mem_clk = 0, sm_clk = 0, sm_count = 0;
	cudaDeviceGetAttribute(&major, cudaDevAttrComputeCapabilityMajor, 0);
	cudaDeviceGetAttribute(&minor, cudaDevAttrComputeCapabilityMinor, 0);
	cudaDeviceGetAttribute(&bus_width, cudaDevAttrGlobalMemoryBusWidth, 0);
	cudaDeviceGetAttribute(&mem_clk, cudaDevAttrMemoryClockRate, 0);
	cudaDeviceGetAttribute(&sm_clk, cudaDevAttrClockRate, 0);
	cudaDeviceGetAttribute(&sm_count, cudaDevAttrMultiProcessorCount, 0);

	if (sm_major) *sm_major = major;
	if (sm_minor) *sm_minor = minor;
	if (bus_width_bits) *bus_width_bits = bus_width;
	if (mem_clock_khz) *mem_clock_khz = mem_clk;

	// Memory bandwidth formula: (bus_width_bits / 8) * mem_clock_hz * 2 (effective rate)
	double clock_hz = (double)mem_clk * 1000.0;
	double bus_bytes = (double)bus_width / 8.0;
	double bw = 2.0 * bus_bytes * clock_hz / 1e9;

	// Architecture-specific known peak saturation figures for Ada Lovelace (sm_89)
	if (major == 8 && minor == 9) {
		if (bus_width >= 384 && bw < 800.0) bw = 1008.0; // RTX 4090
		else if (bus_width >= 256 && bw < 500.0) bw = 716.8; // RTX 4080
		else if (bus_width == 192 && bw < 350.0) bw = 504.2; // RTX 4070 Ti
		else if (bw < 100.0) bw = 716.8;
	} else if (bw < 50.0) {
		bw = 716.8; // Safe default
	}
	if (peak_bandwidth_gb_s) *peak_bandwidth_gb_s = bw;

	// Theoretical Dense BF16 Tensor TFLOPs:
	double sm_clock_ghz = (double)sm_clk / 1e6;
	double tflops = (double)sm_count * 512.0 * sm_clock_ghz / 1000.0;
	if (major == 8 && minor == 9 && tflops < 30.0) {
		if (sm_count >= 128) tflops = 82.58; // RTX 4090
		else if (sm_count >= 76) tflops = 48.74; // RTX 4080
		else if (sm_count >= 46) tflops = 35.30; // RTX 4070
		else tflops = 48.74;
	} else if (tflops < 10.0) {
		tflops = 48.74; // Safe default
	}
	if (peak_dense_tflops) *peak_dense_tflops = tflops;
}

extern "C" void* create_cuda_event() {
	cudaEvent_t evt = nullptr;
	if (cudaEventCreate(&evt) == cudaSuccess) {
		return (void*)evt;
	}
	return nullptr;
}

extern "C" void destroy_cuda_event(void* event) {
	if (event) {
		cudaEventDestroy((cudaEvent_t)event);
	}
}

extern "C" void record_cuda_event(void* event, void* stream) {
	if (event) {
		cudaEventRecord((cudaEvent_t)event, (cudaStream_t)stream);
	}
}

extern "C" void sync_cuda_event(void* event) {
	if (event) {
		cudaEventSynchronize((cudaEvent_t)event);
	}
}

extern "C" float elapsed_cuda_events_ms(void* start_event, void* stop_event) {
	float ms = 0.0f;
	if (start_event && stop_event) {
		cudaEventElapsedTime(&ms, (cudaEvent_t)start_event, (cudaEvent_t)stop_event);
	}
	return ms;
}

