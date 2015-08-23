using namespace std;

#include <map>
#include <queue>
#include <stack>

#include "../headers/problem.h"
#include "../headers/problems.h"
#include "../headers/decompose_beginend.h"
#include "../int/BigInteger.hh"

#ifndef GPU_H_
#define GPU_H_

#define MAX_SIZE_INPUTQUEUE_TO_GPU	10

class pbab;

typedef struct {
	int couts_somme;
	int permutation[MAX_NBJOBS];
	int limite1, limite2;
	int depth;
	unsigned long long int father;
	int begin_end;
} problem_d;

typedef struct CUstream_st *cudaStream_t;

class problemPool {
public:
	raw_bb_problem* children_d;
	int* children_bounds;
	int children_size;
	problemPool();
	~problemPool();
};


class gpu {
	comparator_depth sortByDepth;
	comparator_cost_desceding sortDescendingByCost;
public:

	pbab* pbb;

	char johnson;
	char temps;
	char divergence;

	int time_seed;

	int used_device;

	int nb_block;
	int block_size;

	int pool_size;
	gpu(pbab* pbb, int nb_block, int block_size, int available_devices,
			char johnson, char temps, char divergence);

	void init();

	int* bounds_d[MAX_SIZE_INPUTQUEUE_TO_GPU];
	int* bounds_h[MAX_SIZE_INPUTQUEUE_TO_GPU];
	raw_bb_problem* pool_to_evaluate_d[MAX_SIZE_INPUTQUEUE_TO_GPU];
	raw_bb_problem* pool_to_evaluate_h[MAX_SIZE_INPUTQUEUE_TO_GPU*2];
	cudaStream_t stream[MAX_SIZE_INPUTQUEUE_TO_GPU];

	int current_index_gpu;
	int starting_index_gpu;

	int slice;

	int get_available_devices();

	void init_tabs();

	void problem_to_problem_d(problem &pb, problem_d & pb_d);
	void problem_d_to_problem(problem_d &pb_d, problem & pb);

	void evaluate_on_gpu_asynchronous(problems &src, problems&dest);

	void allocate_on_device();
	void copy_to_device();
	void cuda_Bind();
	void cuda_UnBind();

	//
	bool decompose_on_cpu(problems*);
	void prune(problemPool*);
	int* calculate_bounds_in_gpu(problemPool*);

	//
	deque<problemPool*> readyPoolToGPU;

	// <problems, boundValues>
	deque<problemPool*> outputPoolOfGPU;

	//
	int synchronize_counter_cpu_gpu;

	/**CPU Prune+Decmpose**/
	void prune_shareOutputGPU(problemPool*);
	void prune(raw_bb_problem*, int num_problems);
};

typedef struct {
	int device_number;
	gpu* _gpu;
} args_struct;

#endif /* GPU_H_ */
