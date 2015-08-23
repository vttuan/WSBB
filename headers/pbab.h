#ifndef PBAB_H
#define PBAB_H

using namespace std;
#include <pthread.h>
#include <semaphore.h>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/arguments.h"
#include "tree.h"
#include "solutions.h"
#include "works.h"
#include "work_stealing/node.h"

class peer;
class peers;
class instance_abstract;
class bound_abstract;
class eliminate_abstract;
class decompose_abstract;
class decompose_beginend;
class select_abstract;
//class tree;
//class solutions;
//class works;
class work;
class weights;
class ttime;
class problem;
class gpu;
class explorer;

class select_worst;

class pbab {
public:
	int size;
	peer*ndx, *pr;

	instance_abstract*instance;
	bound_abstract*bound;
	eliminate_abstract*eliminate;
	decompose_beginend*decompose;
	select_abstract*select;

	gpu *gp;
	tree *tr;
	explorer*xplr;
	solutions* isltns;
	solutions* wsltns;
	problem*sltn;
	works*wrks;
	weights* wghts;
	ttime*ttm;

	pbab(int nb_block, int block_size, int available_devices,
	     eliminate_abstract*_eliminate, decompose_beginend*_decompose,
	     instance_abstract*_instance, bound_abstract*_bound,
	     tree* _tr = NULL);

	void run();
	void display();

	void save();
	void init();

	int cpu_operation;

	work* divideWork(work*);
	void setInterval(BigInteger begin, BigInteger end);
	void setWorkerState(int);
	void getGPULock();
	void releaseGPULock();

	int betterSolutionState;
	int workerState;
	int gpuWorkerState;
	pthread_mutex_t to_do_gpu_mutex;
	pthread_cond_t conditionVarTodoForShare;
	Node* node;
};

#endif

