#ifndef GPU_KERNEL_CUH
#define GPU_KERNEL_CUH

#include <cstddef>

struct Grid {
	int thread;
	int block;
	int index;

	inline Grid(size_t output_dim) : thread(256), index(0) {
		block = static_cast<int>((output_dim + thread - 1) / thread);
	}
};

#endif // GPU_KERNEL_CUH
