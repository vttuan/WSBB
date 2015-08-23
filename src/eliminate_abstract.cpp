//========================================================================================================================================
//========================================================================================================================================
//========================================================================================================================================
using namespace std;
#include <iostream>
#include <sstream>
#include <functional>
#include <fstream>
#include <list>
#include <queue>
#include <string>
#include <pthread.h>
#ifndef ELIMINATE_ABSTRACT_CPP
#define problem_CPP
#include "problem.cpp"
#define problemS_CPP
#include "problems.cpp"
#endif 
//========================================================================================================================================
//========================================================================================================================================
//========================================================================================================================================
#ifndef ELIMINATE_ABSTRACT_H
#define ELIMINATE_ABSTRACT_H
class problem;

struct eliminate_abstract: public std::binary_function<raw_bb_problem*, raw_bb_problem*, bool> {
	virtual bool operator()(raw_bb_problem*n1, raw_bb_problem*n2) const =0;// return (n1 eliminate n2)
};

//extern eliminate_abstract* eliminate;

#endif

#ifndef ELIMINATE_ABSTRACT_CPP
//eliminate_abstract* eliminate;
#endif

