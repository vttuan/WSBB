using namespace std;
#include <iostream>
#include <sstream>
#include <functional>
#include <fstream>
#include <list>
#include <queue>
#include <string>
#include <pthread.h>

#ifndef ELIMINATE_ABSTRACT_H
#define ELIMINATE_ABSTRACT_H
class problem;

struct eliminate_abstract:public std::binary_function<raw_bb_problem*, problem*, bool>
{virtual bool operator()(raw_bb_problem*n1, raw_bb_problem*n2) const =0 ;// return (n1 eliminate n2)
};

//extern eliminate_abstract* eliminate;

#endif


