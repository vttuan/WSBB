using namespace std;
#include <sstream>

#ifndef DECOMPOSE_ABSTRACT_H
#define DECOMPOSE_ABSTRACT_H
class problems;
class problem;
class pbab;
class bound_abstract;

struct decompose_abstract {
	pbab*pbb;
	virtual problems& operator()(problem& n) const =0;

	bound_abstract*bound;
	virtual void set_bound(bound_abstract*_bound)=0;
};

#endif
