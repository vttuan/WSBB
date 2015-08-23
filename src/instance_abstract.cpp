using namespace std;
#include <sstream>

#ifndef INSTANCE_ABSTRACT_H
#define INSTANCE_ABSTRACT_H
struct instance_abstract
{
	int size;
	union {
		stringstream*data;
		int*donnee;
	};
};

#endif



