using namespace std;

#include "../headers/problem.h"
#include "../headers/select_abstract.h"
#include "../headers/select_worst.h"

bool select_worst::operator()(raw_bb_problem const* n1, raw_bb_problem const* n2) const {
	if (n1->couts_somme != n2->couts_somme)
		return (n2->couts_somme < n1->couts_somme);
	else
		return (n1->job > n2->job);
}

