using namespace std;
#include <iostream>
#include <sstream>
#include "../headers/weights.h"
#include "../headers/work.h"
#include "../headers/bound_abstract.h"
#include "../headers/problems.h"
#include "../headers/instance_abstract.h"
#include "../headers/pbab.h"
#include "../headers/problem.h"

#define BEGIN_ORDER 0
#define END_ORDER 1

int aaaa=0;

problem::problem()
{
	aaaa++;
}

problem::problem(const problem& p)
{
	aaaa++;
	pbb = p.pbb;
	couts_somme= p.couts_somme;
	for (int i = 0; i < C; i++)	couts[i] = p.couts[i];
	for (int j = 0; j < p.pbb->size; j++)	permutation[j] = p.permutation[j];
	for (int d = 0; d < p.pbb->size + 1; d++)	ranks[d] = p.ranks[d];
	limite1 = p.limite1;
	limite2 = p.limite2;
	depth = p.depth;
	job = p.job;
	begin_end = p.begin_end;
}

problem::~problem()
{
	aaaa--;
}

problem::problem(pbab*pbb, int limite1, int limite2, int depth, int permutation[MAX_NBJOBS], int begin_end, int cost)
{
	aaaa++;
	this->pbb = pbb;
	father = NULL;
	this->limite1 = limite1;
	this->limite2 = limite2;
	this->depth = depth;
	for (int j = 0; j < pbb->size; j++)
		this->permutation[j] = permutation[j];

	this->couts[0] = cost;
	this->couts[1] = 0;
	couts_somme = cost;
	this->begin_end = begin_end;
}

// TUAN, just to create dump father
problem::problem(pbab*pbb, bool flag)
{
	aaaa++;
	this->pbb = pbb;
}


problem::problem(pbab*pbb)
{
	aaaa++;
	this->pbb = pbb;
	father = NULL;
	limite1 = -1;
	limite2 = pbb->size;
	depth = 0;
	for (int j = 0; j < pbb->size; j++)	permutation[j] = j;
	for (int d = 0; d < pbb->size + 1; d++)	ranks[d] = 0;
	for (int j = 0; j < C; j++)	couts[j] = 0;
	couts_somme = 0;
	job = -1;
}

problem::problem(const problem& father, int indice, int begin_end) //pour creer un problem interne.
{
	pbb = father.pbb;
	limites_set(father, begin_end);
	depth_set(father);
	permutation_set(father, indice, begin_end);
	ranks_set(father);
	//couts_set();
}

void problem::limites_set(const problem& father, int begin_end)
{
	limite1 = father.limite1;
	limite2 = father.limite2;

	if (begin_end == BEGIN_ORDER)limite1++;
	else	limite2--;

	if (limite2 - limite1 == 2) limite1++;
}

void problem::depth_set(const problem& father)
{
	if (father.simple())	depth = father.pbb->size;
	else	depth = father.depth + 1;
}

void problem::permutation_set(const problem& father, int indice,int begin_end)
{
	job = father.permutation[indice];
	for (int j = 0; j < father.pbb->size; j++)	permutation[j] = father.permutation[j];
	int tmp_indice =	(begin_end == BEGIN_ORDER) ?	father.limite1 + 1 : father.limite2 - 1;
	int tmp = permutation[tmp_indice];
	permutation[tmp_indice] = permutation[indice];
	permutation[indice] = tmp;
}

void problem::ranks_set(const problem& father)
{
	for (int d = 0; d < father.pbb->size + 1; d++)
		ranks[d] = father.ranks[d];
}

void problem::ranks_depth(int rank)
{
	ranks[depth] = rank;
}

void problem::ranks_depth2(int rank)
{
	ranks[depth - 1] = rank;
}

int problem::ranks_depth()
{
	return ranks[depth];
}

void problem::couts_set()
{
	pbb->bound->bornes_calculer(permutation, limite1, limite2, couts, 999999);
	couts_somme = couts[0] + couts[1];
}
//=============================================================

bool problem::simple() const
{
	return (depth == pbb->size - 2);
}

bool problem::leaf() const
{
	return (depth == pbb->size);
}
//=============================================================
//=============================================================
//=============================================================

ostream& operator<<(ostream& stream, problem& p)
{
	for (int i = 0; i < C; i++)	stream << p.couts[i] << " ";

	stream << p.couts_somme << " ";

	for (int j = 0; j < p.pbb->size; j++)	stream << p.permutation[j] << " ";

	stream << p.limite1 << " " << p.limite2 <<" ";

	stream << p.depth << " ";

	stream << p.job << " ";

	stream << endl;

	return stream;
}

istream& operator>>(istream& stream, problem& p)
{
	for (int i = 0; i < C; i++)	stream >> p.couts[i];

	stream >> p.couts_somme;

	for (int j = 0; j < p.pbb->size; j++)	stream >> p.permutation[j];

	stream >> p.limite1 >> p.limite2;

	stream >> p.depth;

	stream >> p.job;

	return stream;
}

void problem::operator=(const problem& n)
{
	couts_somme = n.couts_somme;

	for (int i = 0; i < C; i++)	couts[i] = n.couts[i];

	for (int j = 0; j < n.pbb->size; j++)	permutation[j] = n.permutation[j];

	for (int d = 0; d < n.pbb->size + 1; d++)	ranks[d] = n.ranks[d];

	limite1 = n.limite1;

	limite2 = n.limite2;

	depth = n.depth;

	job = n.job;

	begin_end = n.begin_end;

	pbb = n.pbb;
}

//=============================================

work* problem::range_get() const
{
	work* i = new work(1, 0, pbb);

	//compute begin

	i->begin = 0;

	for (int d = 0; d <= depth; d++)
	{
		BigInteger b;
		b = ranks[d];
		b *= pbb->wghts->depths[d];
		i->begin += b;
	}

	//compute end

	i->end = i->begin;
	i->end += pbb->wghts->depths[depth];

	//cout<<"1###################################"<<endl;
	//cout<<*i;
	//cout<<"###################################"<<endl;

	return i;
}

//=============================================================
//=============================================================
//=============================================================

void problem::affiche()
{
	work* w = range_get();

	cout << w->begin << " : ";

	for (int i = 0; i <= depth; i++)	cout << ranks[i] << " ";

	cout << endl;
}

void problem::print()
{
	cout << couts_somme << " ";

	for (int j = 0; j < pbb->size; j++)	cout << permutation[j] << " ";

	cout << limite1 << " " << limite2 << " ";

	cout << depth << " ";

	cout << endl;
}

