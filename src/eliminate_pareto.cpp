using namespace std;
#include <iostream>
#include <sstream>
#include <functional>
#include <fstream>
#include <list>
#include <queue>
#include <string>
#include <pthread.h>

#include "../headers/problem.h"
#include "../headers/eliminate_abstract.h"
#include "../headers/problems.h"
#include "../headers/eliminate_pareto.h"


bool eliminate_pareto::operator()(raw_bb_problem*n1, raw_bb_problem*n2) const
{
	if (n2->couts_somme < n1->couts_somme)
		return false;
	return true;
}

