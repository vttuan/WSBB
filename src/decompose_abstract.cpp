using namespace std;
#include <sstream>

#ifndef DECOMPOSE_ABSTRACT_H
#define DECOMPOSE_ABSTRACT_H
class problems;
class problem;

struct decompose_abstract
{
	virtual problems&  operator()(problem& n) const =0; 
};
#endif




