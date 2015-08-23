using namespace std;
#include <iostream>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/problems.h"

class bound_abstract;

#ifndef DECOMPOSE_BEGINEND_H
#define DECOMPOSE_BEGINEND_H

class problems;
class problem;
class pbab;
class decompose_beginend //: public decompose_abstract
{
public:

	pbab*pbb;
	bound_abstract*bound;
	int best;
	problems ns;

	pthread_mutex_t mutex_rank;

	decompose_beginend(long unsigned best = 99999);
	int choise(problems& debut, problems& fin);
	void problems_generate(problems& ns, raw_bb_problem& n, int begin_end);
	void problems_generate(raw_bb_problem& n);
	void leaves_generate(raw_bb_problem& n);
	problems& operator()(raw_bb_problem& n);
	void set_pbab(pbab*pbb);

	void problems_generate2(problems* ns, raw_bb_problem* n, int begin_end);
	void leaves_generate2(raw_bb_problem& n, problems* ns);

	void leaves_generate3(raw_bb_problem* n,raw_bb_problem* ns, int* index);
	void problems_generate3(raw_bb_problem* ns, raw_bb_problem* father,int begin_end, int* index);

	void set_bound(bound_abstract*_bound);

	void problems_generate(raw_bb_problem& n, problems &ns);
	void leaves_generate(raw_bb_problem& n, problems &ns);
};

#endif
