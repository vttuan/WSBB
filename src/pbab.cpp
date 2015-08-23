using namespace std;

#include <pthread.h>
#include <pthread.h>
#include <semaphore.h>

#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/ttime.h"
#include "../headers/decompose_abstract.h"
#include "../headers/decompose_beginend.h"
#include "../headers/weights.h"
#include "../headers/peer.h"
#include "../headers/solutions.h"
#include "../headers/eliminate_abstract.h"
#include "../headers/eliminate_pareto.h"
#include "../headers/tree.h"
#include "../headers/explorer.h"
#include "../headers/bound_abstract.h"
#include "../headers/bound_flowshop.h"
#include "../headers/instance_abstract.h"
#include "../headers/instance_flowshop.h"
#include "../headers/work.h"
#include "../headers/works.h"
#include "../headers/pbab.h"
#include "../headers/gpu.h"

#include "../headers/work_stealing/types.h"

pbab::pbab(int nb_block, int block_size, int used_device,
		   eliminate_abstract*_eliminate, decompose_beginend*_decompose,
		   instance_abstract*_instance, bound_abstract*_bound, tree* _tr)
{
	size = arguments::sizev;

	wghts = new weights(this);

	pr = new peer(arguments::worker_name, arguments::worker_port);
	ndx = new peer(arguments::index_name, arguments::index_port);

	eliminate = _eliminate;
	decompose = _decompose;
	instance = _instance;
	bound = _bound;
	select = NULL;
	ttm = new ttime();

	if (arguments::worker) {
		decompose->set_pbab(this);
		decompose->set_bound(bound);
	}
	bound->set_pbab(this);
	bound->set_instance(instance);

	xplr = new explorer(this);

	if (_tr)
		_tr->set_pbab(this);
	tr = (_tr) ? _tr : new tree(this);

	isltns = new solutions(arguments::directory, this);
	wsltns = new solutions(arguments::directory, this);
	wrks = new works(arguments::directory, this);
	sltn = new problem(this);

	if (arguments::worker)
		gp = new gpu(this, nb_block, block_size, used_device,
						arguments::johnson, arguments::temps,
						arguments::divergence);

	// TUAN
	this->cpu_operation = arguments::cpu_operation;
	this->gpuWorkerState = idle;

	workerState = idle;
	this->to_do_gpu_mutex = PTHREAD_MUTEX_INITIALIZER;
	this->conditionVarTodoForShare = PTHREAD_COND_INITIALIZER;

	this->node = NULL;
}

void pbab::init() {
	isltns->init();
	wrks->init();
}

void pbab::run() {
	xplr->run();
}

void pbab::save() {
	wrks->save();
	isltns->save();
}

work* pbab::divideWork(work* originalWork){
	if (!originalWork->big()){
			return originalWork;
		}

	work*tmp2 = originalWork->divide();
	return tmp2;
}

void pbab::setInterval(BigInteger begin, BigInteger end){
	this->tr->todo->begin = begin;
	this->tr->todo->end = end;
	this->tr->todo->set_size();
	this->tr->todo->set_time();
}

void pbab::setWorkerState(int state) {
	this->workerState = state;
}

void pbab::getGPULock(){
	pthread_mutex_lock(&to_do_gpu_mutex);
}

void pbab::releaseGPULock(){
	pthread_mutex_unlock(&to_do_gpu_mutex);
}
