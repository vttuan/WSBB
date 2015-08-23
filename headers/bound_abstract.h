#include "../headers/problem.h"

#ifndef BOUND_ABSTRACT_H
#define BOUND_ABSTRACT_H
class pbab;
class problem;
class instance_abstract;

struct bound_abstract {
	pbab*pbb;
	virtual void bornes_calculer(int permutation[],
								int limite1, int limite2,
								int*couts, int)=0;
	virtual void bornes_calculer(raw_bb_problem& p)=0;
	void set_pbab(pbab*pbb);

	instance_abstract*instance;
	virtual void set_instance(instance_abstract*_instance)=0;
};
//extern bound_abstract* bound; 

#endif

