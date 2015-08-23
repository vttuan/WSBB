using namespace std;
#include <iostream>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"

#include "../headers/select_abstract.h"
#include "../headers/problem.h"
#include "../headers/problems.h"
#include "../headers/instance_abstract.h"
#include "../headers/bound_abstract.h"
#include "../headers/decompose_abstract.h"
#include "../headers/weights.h"
#include "../headers/pbab.h"
#include "../headers/decompose_beginend.h"


#define BEGIN_ORDER 0
#define END_ORDER 1

decompose_beginend::decompose_beginend(long unsigned best)
{
	this->best = best;
	pthread_mutex_init(&mutex_rank, NULL);
}

void decompose_beginend::problems_generate(problems& ns, raw_bb_problem& n,int begin_end)
{
	ns.clear();

	for(int j=n.limite1+1; j<n.limite2; j++)
	{
		raw_bb_problem *tmp = (raw_bb_problem*) malloc (sizeof (raw_bb_problem));
		pbb->tr->bbCalculator->create_raw_problem3(&n, j, begin_end, tmp);
		pbb->bound->bornes_calculer(*tmp);
		ns.push_back(tmp);
	}
}

int  decompose_beginend::choise(problems& debut,problems& fin) 
{
	int coupe0 = debut.coupe(best);
	int coupe1 = fin.coupe(best);
	if (coupe0 > coupe1) return BEGIN_ORDER;
	if (coupe1 > coupe0) return END_ORDER;
	int sum0 = debut.sum(best);
	int sum1 = fin.sum(best);

	if (sum0 > sum1)return BEGIN_ORDER;
	else return END_ORDER;
}

void decompose_beginend::problems_generate(raw_bb_problem& n)
{
	problems debut, fin;
	problems_generate(debut,n,BEGIN_ORDER);
	problems_generate(fin,n,END_ORDER);

	if (choise(debut,fin) == BEGIN_ORDER)
	{
		ns.insert(ns.begin(),debut.begin(),debut.end());
		fin.empty();
	}
	else
	{
		ns.insert(ns.begin(),fin.begin(),fin.end());
		debut.empty();
	}
}

void decompose_beginend::leaves_generate(raw_bb_problem& n)
{
	raw_bb_problem *tmp;

	tmp = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));
	this->pbb->tr->bbCalculator->create_raw_problem3(&n,n.limite1+1,BEGIN_ORDER, tmp);
	tmp->depth = this->pbb->size;
	tmp->begin_end = BEGIN_ORDER;
	pbb->bound->bornes_calculer(*tmp);
	ns.push_back(tmp);

	raw_bb_problem *tmp1;
	tmp1 = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));
	this->pbb->tr->bbCalculator->create_raw_problem3(&n,n.limite1+2,BEGIN_ORDER, tmp1);
	tmp1->depth = this->pbb->size;
	tmp1->begin_end = BEGIN_ORDER;
	pbb->bound->bornes_calculer(*tmp1);
	ns.push_back(tmp1);

}

problems& decompose_beginend::operator()(raw_bb_problem& n)
{
	if(this->pbb->tr->bbCalculator->simple(&n))
	{
		ns.clear();
		leaves_generate(n);
		ns.ranks2();
	}
	else
	{
		ns.clear();
		problems_generate(n);
		ns.ranks();
	}

	return ns;
}

void decompose_beginend::set_pbab(pbab*pbb)
{
	this->pbb = pbb;
}

void decompose_beginend::problems_generate3(raw_bb_problem* child, raw_bb_problem* father,int begin_end, int* index)
{
	for(int j = father->limite1 + 1 ; j < father->limite2 ; j++)
	{
		(*index)++;

		this->pbb->tr->bbCalculator->create_raw_problem3(father,j,begin_end, (child + *index));

	}
}


void decompose_beginend::leaves_generate3(raw_bb_problem* n,raw_bb_problem* child, int* index)
{
	(*index)++;

	this->pbb->tr->bbCalculator->create_raw_problem3(n,n->limite1+1,BEGIN_ORDER, (child + *index));
	child->depth = this->pbb->size;
	child->begin_end = BEGIN_ORDER;

	(*index)++;
	this->pbb->tr->bbCalculator->create_raw_problem3(n,n->limite1+2,BEGIN_ORDER, (child + *index));
	child->begin_end = BEGIN_ORDER;
	child->depth = this->pbb->size;
}

void decompose_beginend::set_bound(bound_abstract*_bound)
{
	bound = _bound;
}

void decompose_beginend::problems_generate(raw_bb_problem& n,problems &ns)
{
	problems debut, fin;

	problems_generate(debut,n,BEGIN_ORDER);
	problems_generate(fin,n,END_ORDER);

	if (choise(debut,fin) == BEGIN_ORDER)
	{
		ns.insert(ns.begin(),debut.begin(),debut.end());
		fin.empty();
	}
	else
	{
		ns.insert(ns.begin(),fin.begin(),fin.end());
		debut.empty();
	}
}

