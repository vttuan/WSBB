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

#ifndef TREE_H
#define TREE_H

class weights;
class problem;
class work;
class works;
class pbab;
class gpu;
class problemPool;


#define MAX_NBJOBS 20
#define MAX_NB_MACHINES 20
#define DIV_JOHNSON 1
#define MAX_SOMME 190

#define STRATEGY_DEPTH 'd'
#define STRATEGY_BEST 'b'
#define STRATEGY_RANDOM 'r'
#define STRATEGY_HOMOGENE 'h'
#define STRATEGY_PILE 'p'

/*
 * Define which operation CPU cluster will do
 */
#define CPU_WORKSTEALING		0	// For both CPU and GPU cluster, workstealing
#define	CPU_DECOMPOSE			1
#define	CPU_PRUNE_DECOMPOSE		2
#define	CPU_NORMAL_EXPLORE		3
#define CPU_ADAPTIVE			4
#define	CPU_NOT_DEFINED			-1

//=================================================
class comparator_depth {
public:
	bool operator()(raw_bb_problem const* n1, raw_bb_problem const* n2);
};

class comparator_cost_desceding {
public:
	bool operator()(raw_bb_problem const* n1, raw_bb_problem const* n2);
};

class comparator_best {
public:
	bool operator()(raw_bb_problem const* n1, raw_bb_problem const* n2);
};

//=================================================

class tree {

private:

	int strategy;

	priority_queue<raw_bb_problem*, vector<raw_bb_problem*>, comparator_depth> depth;
	priority_queue<raw_bb_problem*, vector<raw_bb_problem*>, comparator_best> best;
	stack<raw_bb_problem*> pile;

	bool passed;
	pbab*pbb;

	long unsigned int took;
	long unsigned int inserted_problems;
	long unsigned int improved;

public:

	work*todo, *todo_com, *todoForShare;
	deque<raw_bb_problem*> babTreeForShare;
	deque<problemPool*> outputGPUForShare;

	deque<cpu_raw_bb_problem*> babProblemsForPrune;

	bool emptyFlagOfIntervalForShare;

	long unsigned int bounded;
	int iteration;
	int best_pool;
	int cout;
	int lost;

	bb_problem_calculator* bbCalculator;
	tree(char strategy);
	tree(pbab*pbb);
	~tree();
	void set_pbab(pbab*_pbb);

	void init();
	void insert(raw_bb_problem*p);
	void insert(vector<raw_bb_problem*>& ns);
	void insert2(vector<raw_bb_problem*>& ns);

	void improve(raw_bb_problem* pr);
	raw_bb_problem* take();
	int size();
	void push(raw_bb_problem*p);
	bool empty();
	raw_bb_problem* top();
	void pop();
	void display();
	void set_best();
	void set_depth();
	void set_random();
	void set_homogene();
	void set_pile();
	void set_strategy(char strategy);

	void clear();
	void add_root();
	void add_inteval();

	void branch(raw_bb_problem* pr);
	bool bound(raw_bb_problem* pr);

	void add_bounded(long unsigned int _bounded);

	long unsigned int inserted_problem;

	BigInteger explored_leaves;
	BigInteger cpu_explored_leaves;

	long cpu_explored_nodes;
	long boundedNodesDontExplore;
	long prunedNodes;
	pthread_mutex_t bab_tree_mutex;
	pthread_mutex_t bab_share_tree_mutex;


	/*
	 *  GPU+CPU CLUSTER
	 */

	// Workstealing
	void cpuShareProblems();
	void gpuShareProblems();
};

#endif

