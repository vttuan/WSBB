using namespace std;
#include <vector>
#include <iostream>
#include <algorithm>
#include "../headers/problem.h"
#include "../headers/pbab.h"
#include "../headers/select_abstract.h"
#include "../headers/select_worst.h"
#include "../headers/problems.h"

#include <pthread.h>

int problems::sum(int best)
{
	int retour = 0;

	for (iterator i = begin(); i != end(); ++i)
		if ((*i)->couts_somme < best)
			retour += (*i)->couts_somme;

	return retour;
}

int problems::coupe(int best)
{
	int retour = 0;

	for (iterator i = begin(); i != end(); ++i)
		if ((*i)->couts_somme >= best)
			retour++;

	return retour;
}

void problems::empty()
{
	for (iterator i = begin(); i != end(); ++i){
		if (*i != NULL){
			free(*i);
		}
	}

	clear();
}

void problems::ranks()
{
	sort(begin(), end(), select2);

	int rank = 0;

	for (reverse_iterator i = rbegin(); i != rend(); ++i)	bbCalculator->ranks_depth(*i, rank++);
}

void problems::ranks2()
{
	sort(begin(), end(), select2);

	int rank = 0;

	for (reverse_iterator i = rbegin(); i != rend(); ++i)	bbCalculator->ranks_depth2(*i, rank++);
}

bool problems::push_back1(raw_bb_problem *p, int max)
{
	if (!size()) size1 = 0;

	int fils = 2 * (MAX_NBJOBS - p->depth);

	if ( (size1 + fils) < max )
	{
		push_back(p);
		size1 += fils;
		return true;
	}
	else
		return false;
}
