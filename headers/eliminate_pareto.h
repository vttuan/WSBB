using namespace std;
#include <iostream>
#include <sstream>
#include <functional>
#include <fstream>
#include <list>
#include <queue>
#include <string>
#include <pthread.h>

#ifndef ELIMINATE_PARETO_H
#define ELIMINATE_PARETO_H
class problem;

struct eliminate_pareto:public eliminate_abstract 
{ bool operator()(raw_bb_problem*n1, raw_bb_problem*n2) const;
};


#endif

