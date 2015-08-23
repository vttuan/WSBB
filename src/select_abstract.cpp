#define problem_CPP
#include "problem.cpp"

#ifndef SELECT_ABSTRACT_H
#define SELECT_ABSTRACT_H
class select_abstract
{
	public:
		virtual bool operator()(problem const* n1, problem const* n2) const =0;
};

extern select_abstract* slct;

#endif


