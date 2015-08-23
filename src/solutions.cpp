
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
#include "../headers/pbab.h"
#include "../headers/solutions.h"

pthread_mutex_t optimalSolutionMutex = PTHREAD_MUTEX_INITIALIZER;
solutions::solutions(string directory, pbab*pbb) 
{
	this->pbb = pbb;
	this->directory = directory;
}

void solutions::init()
{
	ifstream stream((directory + "bab.solutions").c_str());

	if (stream != NULL)
	{
		this->readOptimalSolution(stream, *this);
		stream.close();
	}
}

bool solutions::inserable(raw_bb_problem*n)
{
	for (iterator i = begin(); i != end(); ++i)
		if ((*pbb->eliminate)(&(*i), n))	return false;

	return true;
}

bool solutions::insert(raw_bb_problem* s)
{
	pthread_mutex_lock(&optimalSolutionMutex);
	if (changed = inserable(s))
	{
		// Update tr->cout to the cost of the optimal solution
		// tr->cout is the one used to prune branch and bound tree
		this->pbb->tr->improve(s);

		this->pbb->tr->bbCalculator->print(s);
		this->save();

		for (iterator i = begin(); i != end(); ++i)
			if ((*pbb->eliminate)(s, &(*i)))		erase(i--);

		push_back(*s); //duplication du noeud donc.

	}
	pthread_mutex_unlock(&optimalSolutionMutex);
	return changed;
}

//========================================================================================================================================

void solutions::readOptimalSolution(istream& stream, solutions& s)
{
	int taille;
	stream >> taille;
	bool optimal_solution_found = false;

	for (int i = 0; i < taille; i++)
	{
		raw_bb_problem *tmp = (raw_bb_problem*) malloc (sizeof(raw_bb_problem));
		stream >> *tmp;

		optimal_solution_found = s.insert(tmp);
	}

	// set this flag to broadcast new optimal solution to my neighbors
	if (optimal_solution_found)
		this->pbb->betterSolutionState = 1;
}

ostream& operator<<(ostream& stream, solutions& s)
{
	stream << s.size() << " " << endl;

	for (solutions::iterator i = s.begin(); i != s.end(); ++i)	stream << *i;

	return stream;
}

void solutions::save()
{
//	if (!changed)	return;

//	ofstream f((directory + "bab.solutions").c_str());

	ofstream f;
	f.open((directory + "bab.solutions").c_str(),ofstream::app);
	f << *this;
	f.close();
//	changed = false;
}

