
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
#include <sys/times.h>
#include <limits.h>
#include <algorithm>

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
#include "../headers/works.h"


bool comparator_depth::operator()(raw_bb_problem const* n1, raw_bb_problem const* n2)
{
	for (int d = 0; d < MAX_NBJOBS + 1; d++)
	{
		if (n1->ranks[d] < n2->ranks[d])return false;
		if (n1->ranks[d] > n2->ranks[d])return true;
	}

	return true;
}

bool comparator_cost_desceding::operator()(raw_bb_problem const* n1, raw_bb_problem const* n2) //return  (n1>n2)
{
	return (n1->couts[0] >= n2->couts[0]);
}

bool comparator_best::operator()(raw_bb_problem const* n1, raw_bb_problem const* n2)
{
	return (n1->couts_somme > n2->couts_somme);
}

int tree::size()
{
	switch (strategy)
	{
		case STRATEGY_DEPTH:return depth.size();
		case STRATEGY_BEST:return best.size();
		case STRATEGY_PILE:return pile.size();
		default:std::cout << "Undeended strategy";
		exit(1);
	}
}

void tree::push(raw_bb_problem* p)
{
	switch (strategy)
	{
		case STRATEGY_DEPTH:	depth.push(p);break;
		case STRATEGY_BEST:		best.push(p);break;
		case STRATEGY_PILE:		pile.push(p);break;
		default:std::cout << "Undeended strategy";	exit(1);
	}
}

bool tree::empty()
{
	switch (strategy)
	{
		case STRATEGY_DEPTH:return depth.empty();
		case STRATEGY_BEST:return best.empty();
		case STRATEGY_PILE:return pile.empty();
		default:std::cout << "Undeended strategy";	exit(1);
	}
}

raw_bb_problem* tree::top()
{
	switch (strategy)
	{
		case STRATEGY_DEPTH: return depth.top();
		case STRATEGY_BEST:	return best.top();
		case STRATEGY_PILE:	return pile.top();
		default: std::cout << "Undeended strategy";	exit(1);
	}
}

void tree::pop()
{
	raw_bb_problem *p;

	switch (strategy)
	{
		case STRATEGY_DEPTH:depth.pop();break;
		case STRATEGY_BEST:	best.pop();	break;
		case STRATEGY_PILE:	pile.pop();	break;
		default:	std::cout << "Undeended strategy";	exit(1);
	}
}

tree::tree(char _strategy)
{
	strategy = _strategy;
	bab_tree_mutex = PTHREAD_MUTEX_INITIALIZER;
	emptyFlagOfIntervalForShare = false;

	switch (strategy)
	{
		case STRATEGY_DEPTH:	std::cout << "Tree Stratergy: DEPTH" << strategy << std::endl; break;
		case STRATEGY_BEST:		std::cout << "Tree Stratergy: BEST" << strategy << std::endl; break;
		case STRATEGY_PILE:		std::cout << "Tree Stratergy: PILE" << strategy << std::endl; break;
		default:		std::cout << "Undeended strategy"; break;
	}
}

tree::tree(pbab*_pbb)
{
	set_pbab(_pbb);
}

void tree::set_pbab(pbab*_pbb)
{
	took = 0;
	inserted_problem = 0;
	inserted_problems = 0;
	improved = 0;
	explored_leaves = 0;
	cpu_explored_leaves = 0;
	boundedNodesDontExplore = 0;
	prunedNodes = 0;
	cpu_explored_nodes = 0;
	lost = 0;

	bounded = 0;
	passed = false;
	pbb = _pbb;
	todo = new work(pbb);
	todo_com = new work(pbb);
	todoForShare = new work(pbb);
	cout = INT_MAX;
	cout = arguments::costv;

	this->bbCalculator = new bb_problem_calculator(this->pbb);
}

void tree::clear()
{
	while (!empty())
	{
		raw_bb_problem* n = take();
		delete n;
	}
}

void tree::add_root()
{
	clear();
	raw_bb_problem *root = bbCalculator->create_raw_problem(pbb);
	bbCalculator->ranks_depth(root, 0);
	push(root);
}

void tree::set_best()
{
	strategy = STRATEGY_BEST;
}

void tree::set_depth()
{
	strategy = STRATEGY_DEPTH;

}

void tree::set_pile()
{
	strategy = STRATEGY_BEST;
}

void tree::set_strategy(char strategy)
{
	switch (strategy)
	{
		case STRATEGY_DEPTH:	set_depth(); 	std::cout << "Tree Stratergy: DEPTH" << strategy << std::endl;
		case STRATEGY_BEST:		set_best();		std::cout << "Tree Stratergy: BEST" << strategy << std::endl;
		case STRATEGY_PILE:		set_pile();		std::cout << "Tree Stratergy: PILE" << strategy << std::endl;
		default:		std::cout << "Undeended strategy";
		exit(1);
	}
}

void tree::gpuShareProblems(){

	if (this->size() == 0)
		return;
	else{
		if (this->size() > 1){
			int half = this->size() / 2;

			for (int i=0; i<half; i++){
				raw_bb_problem *n = this->take();
				if (n){
					this->babTreeForShare.push_back(n);
				}
				else{
					break;
				}
			}
		}
	}
}

void tree::cpuShareProblems(){
	if (this->size() == 0)
		return;
	else{
		if (this->size() > 1){
			int half = (this->size()) / 2;

			for (int i=0; i<half; i++){
				raw_bb_problem *n = this->take();
				if (n){
					this->babTreeForShare.push_back(n);
				}
				else{
					break;
				}
			}
		}
	}
}

bool tree::bound(raw_bb_problem* pr)
{
	return (pr->couts[0] < cout);
}

void tree::branch(raw_bb_problem* pr)
{
	insert((*pbb->decompose)(*pr));
}

raw_bb_problem* tree::take()
{
	raw_bb_problem *n = (empty()) ? NULL : top();

	if (n)	pop();

	took++;

	return n;
}

void tree::insert(raw_bb_problem *p)
{

	inserted_problem++;

	push(p);
}

void tree::insert2(vector<raw_bb_problem*>& ns)
{
	int best = cout;

	for (vector<raw_bb_problem*>::iterator i = (ns.end() - 1); i >= ns.begin(); i--)
		if ((*i)->couts[0] >= best)
		{
			delete (*i);
			continue;
		}
		else
		{
			inserted_problems++;
			push(*i);
		}
}

void tree::insert(vector<raw_bb_problem*>& ns)
{
	int best = cout;

	for (unsigned int i = 0; i < ns.size(); i++)
	{

		push(ns[i]);

	}

}

void tree::improve(raw_bb_problem* pr)
{

	if (cout > pr->couts[0])	{
		cout = pr->couts[0];
		improved++;
	}

}

tree::~tree()
{
	std::cout << "solution fin: " << cout << endl << flush;
	std::cout << "explored leaves: " << explored_leaves + cpu_explored_leaves << endl;
	std::cout << "GPU bounded nodes: " << bounded << endl;
	std::cout << "CPU bounded nodes: " << cpu_explored_nodes << endl;
	std::cout << "bounded nodes dont explore: " << boundedNodesDontExplore << endl;
	std::cout << "pruned nodes: " << prunedNodes << endl;
	std::cout << "lost: " << lost << endl;

	std::cout << "took: " << took << endl;
	std::cout << "improved: " << improved << endl;
	std::cout << "inserted_problem: " << inserted_problem << endl;
	std::cout << "inserted_problems: " << inserted_problems << endl;
}

void tree::add_bounded(long unsigned int _bounded)
{
	bounded += _bounded;
}
