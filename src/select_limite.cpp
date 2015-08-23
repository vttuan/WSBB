using namespace std;

#include "../headers/problem.h"
#include "../headers/select_abstract.h"
#include "../headers/select_limite.h"

bool select_limite::operator()(problem const* n1, problem const* n2) const
{
	if (n1->limite2 != n2->limite2)
		return (n2->limite2 > n1->limite2);
	else
		return (n1->limite1 > n2->limite1);
}


