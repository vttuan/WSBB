#include <list>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <pthread.h>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "../headers/decompose_beginend.h"
#include "../headers/bound_abstract.h"
#include "../headers/bound_flowshop.h"
#include "../headers/arguments.h"
#include "../headers/tree.h"

#ifndef EXPLORER_H
#define EXPLORER_H

#define BEGIN_ORDER 0
#define END_ORDER 1

class weights;
class problem;
class work;
class works;
class pbab;
class gpu;
class tree;

#define MAX_NBJOBS 20
#define MAX_NB_MACHINES 20
#define DIV_JOHNSON 1
#define MAX_SOMME 190


class comparator_homogeneous {
public:
	bool operator()(problem const* n1, problem const* n2);
};

typedef struct {
	int explored;
	unsigned long int time;
	float efficiency;
} pool_efficiency;

class explorer {

	comparator_homogeneous homogeneous;

	int id;

public:
	char type;
	pbab*pbb;

	explorer(pbab*pbb);
	void explore();
	void _explore_gpu();
	void _explore_cpu();
	void _explore_cpu_2();
	void _explore_old();
	void _explore_pool();
	void branch(problem* pr);
	bool bound(problem* pr);

	static void unfold(work& w, tree& tr, decompose_beginend& decompose);

	void fold(work& w);
	void rand_init(work& w);
	void _explore_period(int seconds);
	void _explore_period();
	void run();

	//
	void _explore_cpu_gpu();

	/*
	 *  GPU+CPU CLUSTER
	 */

	// Workstealing
	void shareBranchAndBoundProblem();

	// Prune + Decompose
	void cpu_pruneDecomposeProblem();

	void cpu_explore_decomposing();
	void cpu_decompose_subpblems(problems* fathers, raw_bb_problem** ns, int* numOfProblems);

};
#endif

