using namespace std;
#include <iostream>
#include <sstream>
#include <functional>
#include <fstream>
#include <list>
#include <queue>
#include <string>
#include <pthread.h>

#ifndef SOLUTIONS_H
#define SOLUTIONS_H
class problem;
class problems;
class eliminate_abstract;

class solutions: public list<raw_bb_problem> {
public:
	pbab*pbb;
	string directory;
	bool changed;

	//gestion dominance
	solutions(string directory, pbab*pbb);
	void init();

	void solutions_init(int i);
	bool inserable(raw_bb_problem*n);
	bool insert(raw_bb_problem* s); // insertion ou destruction

	//serialisation
	void save();
	void readOptimalSolution(istream& stream, solutions& s);

};
//istream& operator>>(istream& stream, solutions& s,  pbab *pbb);
ostream& operator<<(ostream& stream, solutions& s);

#endif

