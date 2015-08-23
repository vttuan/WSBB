
using namespace std;

#include <limits.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <functional>
#include <fstream>
#include <list>
#include <queue>
#include <string>
#include <pthread.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/times.h>
#include <limits.h>
#include <algorithm>
#include <sched.h>

#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/ttime.h"
#include "../headers/decompose_beginend.h"
#include "../headers/weights.h"
#include "../headers/peer.h"
#include "../headers/problem.h"
#include "../headers/problems.h"
#include "../headers/solutions.h"
#include "../headers/work.h"
#include "../headers/instance_abstract.h"
#include "../headers/pbab.h"
#include "../headers/tree.h"
#include "../headers/explorer.h"
#include "../headers/gpu.h"
#include "../headers/works.h"

#include "../headers/work_stealing/types.h"

//============================================================================================================
//EXPLORATION===============================================================================================
//============================================================================================================

cudaError cudaerror_t1;

void checkCUDAError1(const char *msg)
{
    cudaError_t err = cudaGetLastError();

    if( cudaSuccess != err)
    {
    	fprintf(stderr, "Cuda error: %s: %s.\n", msg, cudaGetErrorString( err) );
        exit(EXIT_FAILURE);
    }
}

bool comparator_homogeneous::operator()(problem const* n1, problem const* n2)//return  (n1>n2)
{
    if (n1->limite2 != n2->limite2) return (n1->limite2 > n2->limite2);
    else return (n1->limite1 > n2->limite1);
}

explorer::explorer(pbab*_pbb)
{
	pbb = _pbb;

	type = arguments::type;
}

void explorer::shareBranchAndBoundProblem(){
	// if there is someone ask me for work
	if (pbb->node->pendingWorkRequest.size() > 0 && pbb->tr->babTreeForShare.size() == 0){

		long long startFoldingTime = statistics::time_get_in_us();
		// extract an interval to share to CPU
		//
		pthread_mutex_lock(&pbb->tr->bab_share_tree_mutex);

		//
		pbb->tr->gpuShareProblems();

		//
		pthread_mutex_unlock(&pbb->tr->bab_share_tree_mutex);

		long long endFoldingTime = statistics::time_get_in_us();
		statistics::total_GPU_FOLD_time += endFoldingTime - startFoldingTime;
	}
}

void explorer::_explore_cpu()
{

	int cout = 0;

	long startCPUComputingTime = statistics::time_get_in_us();

	while (true)
	{
		cout++;
		raw_bb_problem *n = pbb->tr->take();

		if (!n) break;
		
		if(this->pbb->tr->bbCalculator->leaf(n))
		{


			if (pbb->wsltns->insert(n)){
				pbb->node->sendOptimalSolutionToGPUs(pbb);
			}

			// leaves explored
			pbb->tr->cpu_explored_leaves += pbb->wghts->depths[n->depth];
		}
		else
		{

			if (pbb->wsltns->inserable(n))
			{
				pbb->tr->insert((*pbb->decompose)(*n));
				pbb->tr->cpu_explored_nodes++;
			}
			// prune
			else{
				pbb->tr->boundedNodesDontExplore++;
				pbb->tr->cpu_explored_leaves += pbb->wghts->depths[n->depth];
			}
		}

		free (n);

		/*
		 * share work request here
		 */
		if (pbb->node->pendingWorkRequest.size() > 0 || cout > 250)
			break;
	}

	long endCPUComputingTime = statistics::time_get_in_us();
		statistics::total_CPU_computing_time += endCPUComputingTime - startCPUComputingTime;


	// compute the computing capability of me
	if (pbb->tr->cpu_explored_nodes > 0)
		pbb->node->computing_capability = (double) (statistics::total_CPU_computing_time / (pbb->tr->cpu_explored_nodes));
}

void explorer::_explore_gpu(){

}

void explorer::_explore_cpu_gpu(){
	int pool_size = this->pbb->gp->pool_size;

	pbb->gp->synchronize_counter_cpu_gpu = 0;


	long startCPUComputingTime = statistics::time_get_in_us();

	/*
	 * DECOMPOSE, PREPARE POOL FOR GPU
	 */

	while (pbb->tr->size() > 0 && pbb->gp->readyPoolToGPU.size() < MAX_SIZE_INPUTQUEUE_TO_GPU){

		problems* fathers = new problems();
		bool ok;

		// Collecting problems from branch and bound tree
		while(true)
		{
			raw_bb_problem *p = pbb->tr->take();

			if (p == NULL)	{
				break;

			}else{
				if (pbb->tr->bound(p)){
					ok = fathers->push_back1(p,(pool_size - 1));
					if(!ok)
					{
						pbb->tr->insert(p);
						break;
					}
				}else{
						pbb->tr->explored_leaves += pbb->wghts->depths[p->depth];
						pbb->tr->boundedNodesDontExplore++;
						free (p);
				}
			}
		}

		//
		pbb->tr->bounded += fathers->size();
		if (fathers->size() > 0){
			// decompose the collected problems
			if (pbb->gp->decompose_on_cpu(fathers))
				pbb->gp->synchronize_counter_cpu_gpu++;
		}
		
		fathers->empty();
		delete fathers;

		if (pbb->gp->outputPoolOfGPU.size() > 0)
			break;
	}

	/*****************************************************************/
	/*
	 *  PROCESS OUTPUT OF GPU
	 */
	switch (this->pbb->cpu_operation){
		/*****/
		case CPU_WORKSTEALING:
			if (pbb->gp->readyPoolToGPU.size() > 0){
				//
				long startGPUComputingTime = statistics::time_get_in_us();

				while (!pbb->gp->readyPoolToGPU.empty()){
//					if (VERBOSE)
//						cout << "_explore_gpu: GPU readyPoolToGPU: " << pbb->gp->readyPoolToGPU.size() << endl;
					
					// Wait for the data from GPU
					cudaerror_t1 = cudaStreamSynchronize(pbb->gp->stream[pbb->gp->starting_index_gpu]);
					checkCUDAError1("_explore_gpu");

					//
					problemPool* pp = pbb->gp->readyPoolToGPU.front();

					pp->children_bounds = pbb->gp->bounds_h[pbb->gp->starting_index_gpu];

					//
					pbb->gp->prune(pp);

					//
					pbb->gp->readyPoolToGPU.pop_front();

					//
					delete(pp);
					pbb->gp->starting_index_gpu = (pbb->gp->starting_index_gpu + 1) % MAX_SIZE_INPUTQUEUE_TO_GPU;


				}

				//
				long endGPUComputingTime = statistics::time_get_in_us();
				statistics::total_GPU_computing_time += endGPUComputingTime - startGPUComputingTime;
			}
			break;
		/*****/
//		case CPU_DECOMPOSE:
//
//			break;
//		case CPU_PRUNE_DECOMPOSE:
//
//			if (pbb->node->pendingWorkRequestFromMS.size() > 0 && pbb->tr->outputGPUForShare.size() == 0){
//				int count = pbb->node->pendingWorkRequestFromMS.size();
//
//				while (cout > 0 && pbb->gp->outputPoolOfGPU.size() > 0){
//					cout << "HA: " << pbb->gp->outputPoolOfGPU.size() << endl;
//					cout << "HA1: " << pbb->node->pendingWorkRequestFromMS.size() << endl;
//
//					problemPool* pp = pbb->gp->outputPoolOfGPU.front();
//					//
//					pbb->gp->prune_shareOutputGPU(pp);
//					//
//					pbb->gp->outputPoolOfGPU.pop_front();
//
//					count--;
//				}
//			}else{
//				if (pbb->gp->outputPoolOfGPU.size() > 0){
//						problemPool* pp = pbb->gp->outputPoolOfGPU.front();
//						pbb->gp->outputPoolOfGPU.pop_front();
//
//						//
//						pbb->gp->prune(pp);
//
//						//
//						free(pp->children_d);
//						free(pp->children_bounds);
//						delete pp;
//				}
//			}
//			break;
//		case CPU_NORMAL_EXPLORE: break;
//		case CPU_ADAPTIVE: break;
//		default: cout << "CPU operation NOT_DEFINED" << endl; break;
	}

	//
	long endCPUComputingTime = statistics::time_get_in_us();
	statistics::total_CPU_computing_time += endCPUComputingTime - startCPUComputingTime;

	// compute the computing capability of me
	if (pbb->tr->bounded > 0)
		pbb->node->computing_capability = (double) (statistics::total_CPU_computing_time / (pbb->tr->bounded));

	/*****************************************************************/
	/*
	 * SHARE WORKS TO OTHERS
	 */
}

void * tree_thread(void *_e)
{
	explorer *e = (explorer*) _e;

	if (e->type == 'c')	e->_explore_cpu();
	else if (e->type == 'g')	e->_explore_gpu();

		else std::cout<<"ERROR in tree thread, no CPU, and GPU "<<flush;

	return NULL;
}


void explorer::explore()
{
	tree_thread(this);
}

void explorer::run()
{
	if (arguments::worker)	pbb->xplr->explore();
	else
	{
		pbb->wrks->cout = arguments::costv;
		pbb->init();
		sleep(99999999); //attendre lorsqu'il s'agit du coordinateur
	}
}

void explorer::cpu_decompose_subpblems(problems* fathers, raw_bb_problem **p, int* numOfProblems){
	cout << statistics::time_get_in_second() << ": decompose_on_cpu, parent size: " << fathers->size() << endl;
	cout << statistics::time_get_in_second() << ": decompose_on_cpu, parent size1: " << fathers->size1 << endl;

	raw_bb_problem* tmp = (raw_bb_problem *) malloc(fathers->size1 * sizeof(raw_bb_problem)) ;
	raw_bb_problem* tmp1 = tmp;

	int index = -1;

	for (int l = 0; l < fathers->size() ; l++)
	{
		if (this->pbb->tr->bbCalculator->simple(fathers->at(l)))
			pbb->decompose->leaves_generate3(fathers->at(l),tmp1, &index);
		else
		{
			pbb->decompose->problems_generate3(tmp1,fathers->at(l),BEGIN_ORDER, &index);

			pbb->decompose->problems_generate3(tmp1,fathers->at(l),END_ORDER, &index);
		}

	}

	if (index >= 0){
		*p = tmp;
		*numOfProblems = index + 1;
	}else {
		free(tmp);
		*p = NULL;
		*numOfProblems = 0;
	}
}

/*
 * CPU decompose bab problems and send the results to GPU
 */
void explorer::cpu_explore_decomposing(){

	int pool_size = this->pbb->gp->pool_size;;

	problems* fathers = new problems();
	bool ok;

	while (true){
		raw_bb_problem *n = pbb->tr->take();

		if (!n) break;

		if(this->pbb->tr->bbCalculator->leaf(n))
		{
			if (pbb->tr->bound(n))
				pbb->wsltns->insert(n);
				pbb->node->sendOptimalSolutionToGPUs(pbb);

			// leaves explored
			pbb->tr->cpu_explored_leaves += pbb->wghts->depths[n->depth];
			free(n);
		}
		else
		{
			/*
			 *  prepare to decompose here
			 */
			if (pbb->tr->bound(n))
			{
				ok = fathers->push_back1(n,(pool_size - 1));
				if(!ok)
				{
					pbb->tr->insert(n);
					break;
				}

				pbb->tr->cpu_explored_nodes += fathers->size();
			}
			// prune
			else{
				pbb->tr->cpu_explored_leaves += pbb->wghts->depths[n->depth];
				free(n);
			}
		}
	}
	/*
	 *  decompose the collected problems
	 */
	if (fathers->size() > 0){
		raw_bb_problem* pp = NULL;
		int sizeInElements;
		sizeInElements = 0;

		this->cpu_decompose_subpblems(fathers, &pp, &sizeInElements);
		cout << "AAAAA, sizeInElements: " << sizeInElements << endl;

		// send the decomposed sub problems to GPU
		if (sizeInElements > 0 & pp != NULL){
			pbb->node->sendDecomposedSubProblemsToGPU(sizeInElements, (char*) pp);
			free(pp);
		}else {
			free(pp);
		}

	}
	fathers->empty();
	delete fathers;

}

void explorer::cpu_pruneDecomposeProblem(){
	while (this->pbb->tr->babProblemsForPrune.size() > 0 || this->pbb->tr->size() > 0) {
		// Prune
		while (this->pbb->tr->babProblemsForPrune.size() > 0){
			cpu_raw_bb_problem* recv_problems = this->pbb->tr->babProblemsForPrune.front();

			int num_problems = recv_problems->size_int_byte / sizeof (raw_bb_problem);
			this->pbb->gp->prune(recv_problems->data, num_problems);

			this->pbb->tr->babProblemsForPrune.pop_front();
			free(recv_problems->data);
			free(recv_problems);
		}

		// Decompose + Return the decompose one to GPU
		cpu_explore_decomposing();


	}
}
