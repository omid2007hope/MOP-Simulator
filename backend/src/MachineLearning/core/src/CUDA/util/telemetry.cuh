#ifndef CUDA_TELEMETRY_CUH
#define CUDA_TELEMETRY_CUH

#include "gpu_types.cuh"

#ifdef __cplusplus
extern "C" {
#endif

// Fetches GPU VRAM usage.
CUDA_API void get_gpu_telemetry(size_t* free_mem, size_t* total_mem);

// Fetches detailed GPU hardware specifications and theoretical capabilities.
CUDA_API void get_hardware_specs(char* name_buf, int name_buf_len,
                                 int* sm_major, int* sm_minor,
                                 int* bus_width_bits, int* mem_clock_khz,
                                 double* peak_bandwidth_gb_s, double* peak_dense_tflops);

// Hardware-level CUDA Event Profiling API
CUDA_API void* create_cuda_event();
CUDA_API void destroy_cuda_event(void* event);
CUDA_API void record_cuda_event(void* event, void* stream);
CUDA_API void sync_cuda_event(void* event);
CUDA_API float elapsed_cuda_events_ms(void* start_event, void* stop_event);

#ifdef __cplusplus
}
#endif

#endif // CUDA_TELEMETRY_CUH
