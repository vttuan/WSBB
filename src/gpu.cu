/*
 *  gpu.cpp
 *
 *  Author: imen chakroun
 *
 */

#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <algorithm>
#include <stack>
#include <vector>
#include <iterator>
#include <list>

#include <time.h>
#include <sys/time.h>
#include <iostream>

#include <cutil.h>
#include <cublas.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cutil_inline.h>

#include <shrUtils.h>

#include "../headers/pbab.h"
#include "../headers/peer.h"
#include "../headers/explorer.h"
#include "../headers/tree.h"
#include "../headers/weights.h"
#include "../headers/problems.h"

#include "../headers/bound_flowshop_gpu.h"

#include "../headers/gpu.h"
#include "../headers/bound_abstract.h"
#include "../headers/instance_abstract.h"

#include "../headers/solutions.h"

#include "bound_flowshop_gpu.cu"

#define BEGIN_ORDER 0
#define END_ORDER 1

bool descendingCom(problem const* n1, problem const* n2){
	return (n1->couts_somme >= n2->couts_somme);
}
cudaError cudaerror_t;

void checkCUDAError(const char *msg)
{
    cudaError_t err = cudaGetLastError();

    if( cudaSuccess != err)
    {
    	fprintf(stderr, "Cuda error: %s: %s.\n", msg, cudaGetErrorString( err) );
        exit(EXIT_FAILURE);
    }
}

problemPool::problemPool(){
	this->children_d = NULL;
	this->children_bounds = NULL;
	this->children_size = 0;
}

problemPool::~problemPool(){
}

gpu::gpu(pbab* pbb,int nb_block, int block_size, int used_device, char johnson,char temps, char divergence)
{
    this->johnson = johnson;
    this->temps = temps;
    this->divergence = divergence;
    this->pbb = pbb;
    this->block_size = block_size;
    this->nb_block = nb_block;
    this->used_device = used_device;

}

void gpu::init_tabs()
{
    cout << "Using this device " << used_device <<" " << nb_block <<" " << block_size << endl<<flush;

	switch (used_device)
	{
		case 0: { cudaSetDevice(0); break; }
		case 1: { cudaSetDevice(1); break; }
		case 2: { cudaSetDevice(2);	break; }
		case 3:	{ cudaSetDevice(3);	break; }
		default: cout << "No Cuda Capable Device : Error in gpu::init()" <<endl <<flush ; exit(0);
	}

//	int *bounds = (int *) malloc(nb_block * block_size* sizeof(int)) ;

//	for (int i = 0 ; i < nb_block * block_size ; i++) bounds[i] = 0;

//	pool_to_evaluate = (problem_d *) malloc(nb_block * block_size * sizeof(problem_d)) ;

//	for (int i = 0 ; i < nb_block * block_size; i++) init_problem_device_g(pool_to_evaluate[i]);

	// Create pinned memory in host CPU for ansynchronous copy
	for (int i=0; i<MAX_SIZE_INPUTQUEUE_TO_GPU; i++){

		// Asynchronous
		cudaMallocHost((void**) &pool_to_evaluate_h[i], pool_size * sizeof(raw_bb_problem));
		cudaMallocHost((void**) &bounds_h[i], pool_size * sizeof(int));

		cudaStreamCreate(&stream[i]);

		// Allocate Memory in GPU device
		cudaMalloc( (void**) &pool_to_evaluate_d[i], pool_size * sizeof(raw_bb_problem));
		cudaMalloc( (void**) &bounds_d[i], nb_block * pool_size * sizeof(int));
	}
	current_index_gpu = 0;
	starting_index_gpu = 0;

//	cudaMemcpy(bounds_d, bounds, nb_block * block_size * sizeof(int), cudaMemcpyHostToDevice);
}

void gpu::init()
{
 	(pbb->instance->data)->seekg (0); 
	(pbb->instance->data)->clear ();

   	*(pbb->instance->data)>>nbJob_g;
	*(pbb->instance->data)>>nbMachines_g;

	*(pbb->instance->data)>>time_seed_g;

	tempsJob_g = (int *)malloc( nbMachines_g * nbJob_g * sizeof(int));
	tempsJob_T = (int **)malloc( nbMachines_g * sizeof(int *));

	for(int i = 0; i < nbMachines_g; i++)
	   tempsJob_T[i] = (int *)malloc( nbJob_g * sizeof(int)) ;

	for(int i = 0; i < nbMachines_g; i++)
	  for(int j = 0; j < nbJob_g; j++)
	  {
		  *(pbb->instance->data)>>tempsJob_T[i][j];
		   tempsJob_g[i * nbJob_g + j] = tempsJob_T[i][j];
	  }


	init_tabs();
	init_bound();

	allocate_on_device();

	checkCUDAError("allocation échouée");

	copy_to_device();

	checkCUDAError("copie échouée");

	free_memories();
}

void gpu::problem_to_problem_d(problem& pb,problem_d& pb_d)
{
	for (int l = 0; l < pbb->size; l++) pb_d.permutation[l] = pb.permutation[l];

	pb_d.limite1 = pb.limite1;

	pb_d.limite2 = pb.limite2;

	pb_d.depth = pb.depth;

	pb_d.begin_end = pb.begin_end;

	pb_d.father = (unsigned long long int) pb.father;

}

bool gpu::decompose_on_cpu(problems* fathers)
{
//	if (VERBOSE){
//		cout << statistics::time_get_in_second() << ": decompose_on_cpu, parent size: " << fathers->size() << endl;
//		cout << statistics::time_get_in_second() << ": decompose_on_cpu, parent size1: " << fathers->size1 << endl;
//	}
	

	problemPool *pp = new problemPool();

	raw_bb_problem* tmp = this->pool_to_evaluate_h[current_index_gpu] ;
	int index = -1;

	for (int l = 0; l < fathers->size() ; l++)
	{
		if (this->pbb->tr->bbCalculator->simple(fathers->at(l)))
			pbb->decompose->leaves_generate3(fathers->at(l),tmp, &index);
		else
		{
			pbb->decompose->problems_generate3(tmp,fathers->at(l),BEGIN_ORDER, &index);

			pbb->decompose->problems_generate3(tmp,fathers->at(l),END_ORDER, &index);
		}

	}

	if (index >= 0){
		pp->children_size = index + 1;
		pp->children_d = tmp;

		// Asynchronous Copy and launch in GPU
		cudaerror_t = cudaMemcpyAsync(pool_to_evaluate_d[current_index_gpu], pp->children_d,
						pp->children_size * sizeof(raw_bb_problem), cudaMemcpyHostToDevice, stream[current_index_gpu]);

		checkCUDAError("copie 2 échouée");

		Evaluate_ON_GPU<<<nb_block, block_size, 0, stream[current_index_gpu]>>>(pool_to_evaluate_d[current_index_gpu],
														bounds_d[current_index_gpu],
														nbJob_g,
														nbMachines_g,
														nbborne_g,
														somme_g,
														nbElem_g,
														nbFois_d,
														machine_d,
														tabJohnson_d,
														tempsJob_d,
														tempsLag_d,
														minTempsArr_d,
														ordoSomme_d,
														minTempsDep_d,
														pp->children_size,
														time_seed_g);
		cudaerror_t = cudaMemcpyAsync(bounds_h[current_index_gpu], bounds_d[current_index_gpu], pp->children_size * sizeof(int),
							cudaMemcpyDeviceToHost, stream[current_index_gpu]);

		checkCUDAError("copie 3 échouée");

		pbb->gp->readyPoolToGPU.push_back(pp);
		
		current_index_gpu = (current_index_gpu + 1) % MAX_SIZE_INPUTQUEUE_TO_GPU;

//		if (VERBOSE)
//			cout << statistics::time_get_in_second() << ": decompose_on_cpu, readyPool: " << pbb->gp->readyPoolToGPU.size() << endl;
		
		return true;
	}else{
//		free(pp->children_d);
		delete pp;
		
//		if (VERBOSE)
//			cout << statistics::time_get_in_second() << ": decompose_on_cpu, readyPool1: " << pbb->gp->readyPoolToGPU.size() << endl;
		
		return false;
	}
}

void gpu::prune(problemPool* pp){

//	if (VERBOSE){
//		cout << statistics::time_get_in_second() << ": Before prune, tree size: " << this->pbb->tr->size() << endl;
//		cout << statistics::time_get_in_second() << ": Before prune, num_elements: " << pp->children_size << endl;
//	}

	vector <problem*> fathers;
	int index = 0;
	while (index < pp->children_size){

			// assigned cost to b&b problems
			pp->children_d[index].couts_somme = pp->children_bounds[index];
			pp->children_d[index].couts[0] = pp->children_bounds[index];
			pp->children_d[index].couts[1] = 0;

			problem* father_problem = new problem(this->pbb, true);

			if (pp->children_d[index].begin_end == BEGIN_ORDER)
			{
				father_problem->debut.push_back((pp->children_d + index));
			}
			else
			{

				father_problem->fin.push_back((pp->children_d + index));

			}

			int consecutive_subproblems = (MAX_NBJOBS + 1 - pp->children_d[index].depth) * 2;
			consecutive_subproblems--;

			// continue add all subproblems to this parent
			for (int jj = 1; jj <= consecutive_subproblems; jj++){
				index++;

				// assigned cost to b&b problems
				pp->children_d[index].couts_somme = pp->children_bounds[index];
				pp->children_d[index].couts[0] = pp->children_bounds[index];
				pp->children_d[index].couts[1] = 0;

				if (pp->children_d[index].begin_end == BEGIN_ORDER)
				{
					father_problem->debut.push_back((pp->children_d + index));
				}
				else
				{
					father_problem->fin.push_back((pp->children_d + index));
				}

			}
			index++;

			// add this father problem to a list
			fathers.push_back(father_problem);
	}

//	if (VERBOSE)
//		cout << statistics::time_get_in_second() << ": prune, father size0: " << fathers.size() << endl;

	for(vector<problem*>::iterator i = fathers.begin(); i != fathers.end() ; ++i) //je parcours les pères
	{
		if ( pbb->decompose->choise( (*i)->debut,(*i)->fin ) == BEGIN_ORDER )
		{

			(*i)->debut.ranks();

			for(vector<raw_bb_problem*>::iterator j = (*i)->debut.begin(); j < (*i)->debut.end() ; ++j)
			{
				if(this->pbb->tr->bbCalculator->leaf(*j))
				{
					if (pbb->wsltns->insert(*j)){
						pbb->node->sendOptimalSolutionToGPUs(pbb);
						pbb->node->sendOptimalSolutionToCPUs(pbb);
						
//						if (VERBOSE){
//							cout << statistics::time_get_in_second() << ": cout: " << pbb->tr->cout << endl;
//							cout << statistics::time_get_in_second() << ": Optimal found: " << *(pbb->wsltns) << endl;
//						}
						
					}


					pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
				}
				else
				{
					if ( pbb->tr->bound(*j)){
						raw_bb_problem* new_problem = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));
						memcpy(new_problem, *j, sizeof(raw_bb_problem));
						pbb->tr->insert(new_problem);
					}
					else {
						pbb->tr->prunedNodes++;
						pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
					}
				}
			}
		}
		else
		{

			(*i)->fin.ranks();
			for(vector<raw_bb_problem*>::iterator j = (*i)->fin.begin(); j < (*i)->fin.end() ; ++j)
			{

				if(this->pbb->tr->bbCalculator->leaf(*j))
				{

					if (pbb->wsltns->insert(*j)){
						pbb->node->sendOptimalSolutionToGPUs(pbb);
						pbb->node->sendOptimalSolutionToCPUs(pbb);

//						if (VERBOSE){
//							cout << statistics::time_get_in_second() << ": cout: " << pbb->tr->cout << endl;
//							cout << statistics::time_get_in_second() << ": Optimal found: " << *(pbb->wsltns) << endl;
//						}
						
					}

					pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
				}
				else
				{
					if ( pbb->wsltns->inserable(*j)){
						raw_bb_problem* new_problem = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));
						memcpy(new_problem, *j, sizeof(raw_bb_problem));
						pbb->tr->insert(new_problem);
					}
					else {
						pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
					}
				}
			}
		}
		(*i)->debut.clear();
		(*i)->fin.clear();
	}

	for(vector<problem*>::iterator i = fathers.begin(); i != fathers.end() ; ++i) //je parcours les pères
	{
		delete (*i);
	}
	fathers.erase(fathers.begin(), fathers.end());

//	if (VERBOSE){
//		cout << statistics::time_get_in_second() << ": prune, father size: " << fathers.size() << endl;
//		cout << statistics::time_get_in_second() << ": prune, tree size: " << pbb->tr->size() << endl;
//		cout << statistics::time_get_in_second() << ": prune, tree size in bytes: " << pbb->tr->size() * sizeof(raw_bb_problem) << endl << endl;
//	}
	
}


void gpu::prune(raw_bb_problem* pp, int num_problems){

	if (VERBOSE){
		cout << statistics::time_get_in_second() << ": Before prune, tree size: " << this->pbb->tr->size() << endl;
		cout << statistics::time_get_in_second() << ": Before prune, num_elements: " << num_problems << endl;
	}

	vector <problem*> fathers;
	int index = 0;

	while (index < num_problems){

			problem* father_problem = new problem(this->pbb, true);

			if (pp[index].begin_end == BEGIN_ORDER)
			{
				father_problem->debut.push_back((pp + index));
			}
			else
			{

				father_problem->fin.push_back((pp + index));

			}

			int consecutive_subproblems = (MAX_NBJOBS + 1 - pp[index].depth) * 2;
			consecutive_subproblems--;

			// continue add all subproblems to this parent
			for (int jj = 1; jj <= consecutive_subproblems; jj++){
				index++;

				if (pp[index].begin_end == BEGIN_ORDER)
				{
					father_problem->debut.push_back((pp + index));
				}
				else
				{
					father_problem->fin.push_back((pp + index));
				}

			}
			index++;

			// add this father problem to a list
			fathers.push_back(father_problem);
	}
	
	if (VERBOSE)
		cout << statistics::time_get_in_second() << ": prune, father size0: " << fathers.size() << endl;

	for(vector<problem*>::iterator i = fathers.begin(); i != fathers.end() ; ++i) //je parcours les pères
	{
		if ( pbb->decompose->choise( (*i)->debut,(*i)->fin ) == BEGIN_ORDER )
		{

			(*i)->debut.ranks();

			for(vector<raw_bb_problem*>::iterator j = (*i)->debut.begin(); j < (*i)->debut.end() ; ++j)
			{
				
				if(this->pbb->tr->bbCalculator->leaf(*j))
				{

					if (pbb->wsltns->insert(*j)){
						pbb->node->sendOptimalSolutionToGPUs(pbb);
						pbb->node->sendOptimalSolutionToCPUs(pbb);

						if (VERBOSE){
							cout << statistics::time_get_in_second() << ": cout: " << pbb->tr->cout << endl;
							cout << statistics::time_get_in_second() << ": Optimal found: " << *(pbb->wsltns) << endl;
						}
						
					}

					pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
				}
				else
				{
					if ( pbb->tr->bound(*j)){

						raw_bb_problem* new_problem = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));
						memcpy(new_problem, *j, sizeof(raw_bb_problem));
						pbb->tr->insert(new_problem);
					}
					else {
						pbb->tr->prunedNodes++;
						pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
					}
				}
			}
		}
		else
		{

			(*i)->fin.ranks();
			for(vector<raw_bb_problem*>::iterator j = (*i)->fin.begin(); j < (*i)->fin.end() ; ++j)
			{

				if(this->pbb->tr->bbCalculator->leaf(*j))
				{

					if (pbb->wsltns->insert(*j)){
						pbb->node->sendOptimalSolutionToGPUs(pbb);
						pbb->node->sendOptimalSolutionToCPUs(pbb);

						if (VERBOSE){
							cout << statistics::time_get_in_second() << ": cout: " << pbb->tr->cout << endl;
							cout << statistics::time_get_in_second() << ": Optimal found: " << *(pbb->wsltns) << endl;
						}
						
					}

					pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
				}
				else
				{
					if ( pbb->wsltns->inserable(*j)){

						raw_bb_problem* new_problem = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));
						memcpy(new_problem, *j, sizeof(raw_bb_problem));
						pbb->tr->insert(new_problem);
					}
					else {
						pbb->tr->explored_leaves += pbb->wghts->depths[(*j)->depth];
					}
				}
			}
		}
//		(*i)->debut.clear();
//		(*i)->fin.clear();
	}

	for(vector<problem*>::iterator i = fathers.begin(); i != fathers.end() ; ++i)
	{
		delete (*i);
	}
	fathers.erase(fathers.begin(), fathers.end());

	if (VERBOSE){
		cout << statistics::time_get_in_second() << ": prune, father size: " << fathers.size() << endl;
		cout << statistics::time_get_in_second() << ": prune, tree size: " << pbb->tr->size() << endl;
		cout << statistics::time_get_in_second() << ": prune, tree size1 in bytes: " << pbb->tr->size() * sizeof(raw_bb_problem) << endl << endl;
	}
	
}
void gpu::prune_shareOutputGPU(problemPool* pp){
	if (VERBOSE)
		cout << statistics::time_get_in_second() << ": prune_shareOutputGPU, children size: " << pp->children_size << endl;
	
	int index = 0;
	while (index < pp->children_size){

			// assigned cost to b&b problems
			pp->children_d[index].couts_somme = pp->children_bounds[index];
			pp->children_d[index].couts[0] = pp->children_bounds[index];
			pp->children_d[index].couts[1] = 0;

			index++;
	}

	this->pbb->tr->outputGPUForShare.push_back(pp);
	
	if (VERBOSE)
		cout << statistics::time_get_in_second() << ": prune_shareOutputGPU, outputGPUForShare: " << this->pbb->tr->outputGPUForShare.size() << endl;

}
int* gpu::calculate_bounds_in_gpu(problemPool* pp){
	
//	if (VERBOSE)
//		cout << statistics::time_get_in_second() << ": GPU, calculate_bounds_in_gpu: " << pp->children_size << endl;
//
//	int *bounds = (int *) malloc(pp->children_size * sizeof(int)) ;
//
//	cudaerror_t = cudaMemcpy(pool_to_evaluate_d, pp->children_d, pp->children_size * sizeof(raw_bb_problem), cudaMemcpyHostToDevice);
//	checkCUDAError("copie 2 échouée");
//
//
//	Evaluate_ON_GPU<<<nb_block, block_size>>>(pool_to_evaluate_d,
//												bounds_d,
//												nbJob_g,
//												nbMachines_g,
//												nbborne_g,
//												somme_g,
//												nbElem_g,
//												nbFois_d,
//												machine_d,
//												tabJohnson_d,
//												tempsJob_d,
//												tempsLag_d,
//												minTempsArr_d,
//												ordoSomme_d,
//												minTempsDep_d,
//												pp->children_size,
//												time_seed_g);
//
//	cudaerror_t = cudaMemcpy(bounds, bounds_d, pp->children_size * sizeof(int), cudaMemcpyDeviceToHost);
//	checkCUDAError("copie 3 échouée");
//
//	if (VERBOSE){
//		cout << statistics::time_get_in_second() << ": GPU, return bounds: " << bounds[0] << endl;
//		cout << statistics::time_get_in_second() << ": GPU, outputPoolOfGPU: " << this->pbb->gp->outputPoolOfGPU.size() << endl;
//		cout << statistics::time_get_in_second() << ": GPU, outputGPUForShare: " << this->pbb->tr->outputGPUForShare.size() << endl;
//		cout << statistics::time_get_in_second() << ": GPU, Worker state: " << this->pbb->workerState << endl;
//	}
//
//	return bounds;
}

void gpu::allocate_on_device()
{
    cudaMalloc( (void**) &tempsJob_d, nbJob_g * nbMachines_g * sizeof(int));
    cudaMalloc( (void**) &tabJohnson_d, nbJob_g * somme_g * sizeof(int));
    cudaMalloc( (void**) &tempsLag_d, nbJob_g * somme_g * sizeof(int));
    cudaMalloc( (void**) &nbFois_d, somme_g  * sizeof(int));
    cudaMalloc( (void**) &ordoSomme_d, somme_g  * sizeof(int));
    cudaMalloc( (void**) &minTempsArr_d, nbMachines_g * sizeof(int));
    cudaMalloc( (void**) &minTempsDep_d, nbMachines_g * sizeof(int));
    cudaMalloc( (void**) &machine_d, 2 * somme_g  * sizeof(int));
}

void gpu::copy_to_device()
{
    cudaMemcpy(tempsJob_d, tempsJob_g, nbJob_g * nbMachines_g * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(tabJohnson_d, tabJohnson_g, nbJob_g * somme_g * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(tempsLag_d, tempsLag_g, nbJob_g * somme_g * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(nbFois_d, nbFois_g, somme_g  * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(ordoSomme_d, ordoSomme_g, somme_g  * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(minTempsArr_d, minTempsArr_g, nbMachines_g * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(minTempsDep_d, minTempsDep_g, nbMachines_g * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(machine_d, machine_g, 2 * somme_g  * sizeof(int), cudaMemcpyHostToDevice);
}
