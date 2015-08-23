using namespace std;
#include <iostream>
#include <sstream>

#ifndef problem_H
#define problem_H
/*
 Limites:
 --------
 limite1: indice of the last job affected in the begining
 limite2: indice du the first job affected at the end
 Begining: limite1=-1, et limite2=pbb->instance->size;
 End: limite1+1=limite2;
 Depth:
 ------
 depth(problem)= Nbre de jobs affecté
 depth(root)=0
 depth(problem)=pbb->instance->size-1 est inexistant.
 depth(leaf)=pbb->instance->size
 Simple problem:
 ------------
 Un noeud pour lequel il ne reste que deux jobs affecté (avant avant dernier).
 <==>limite2-limite1=3.
 <==> depth=pbb->instance->size-2.
 */
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/problems.h"
#include "../headers/tree.h"
#include "work.h"

#define C 2
class problems;
class pbab;


class problem {
public:

	pbab *pbb;

	int size;

	problems debut;
	problems fin;
	bool set_clear_debut;
	bool set_clear_fin;
	problem* father;

	int begin_end;

	int couts[C];
	int couts_somme;
	int job;
	int permutation[500];
	int ranks[501];
	int limite1, limite2, depth;

	problem(pbab*pbb);
	problem(pbab*pbb, bool);

	problem(pbab*pbb, int limite1, int limite2, int depth, int permutation[MAX_NBJOBS], int begin_end, int cost);
	problem();
	~problem();
	problem(const problem& p);
	problem(const problem& father, int indice, int begin_end);

	void limites_set(const problem& father, int begin_end);
	void depth_set(const problem& father);
	void permutation_set(const problem& father, int indice, int begin_end);
	void ranks_set(const problem& father);
	void ranks_depth(int rank);
	void ranks_depth2(int rank);
	int ranks_depth();
	void couts_set(); //doit etre appellee en dernier
	bool leaf() const;
	bool simple() const;
	//gestion d'workle;
	work* range_get() const;
	//gestion de la sauvegarde (flux);
	//void operator>>(ostream& stream) const;
	//void operator<<(istream& stream);
	void operator=(const problem& n);
	void affiche();
	void print();
};

ostream& operator<<(ostream& stream, problem& p);
istream& operator>>(istream& stream, problem& p);

#endif

