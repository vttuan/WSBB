#include "../headers/bb_problem_calculator.h"
#include "../headers/pbab.h"

bb_problem_calculator::bb_problem_calculator(pbab* pbb){
	this->pbb = pbb;
	cout << "******************" << endl;
}

raw_bb_problem* bb_problem_calculator::create_raw_problem(pbab* pbb){

	raw_bb_problem* p = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));
	p->limite1 = -1;
	p->limite2 = pbb->size;
	p->depth = 0;
	for (int j = 0; j < pbb->size; j++)	p->permutation[j] = j;
	for (int d = 0; d < pbb->size + 1; d++)	p->ranks[d] = 0;
	for (int j = 0; j < 2; j++)	p->couts[j] = 0;
	p->couts_somme = 0;
	p->job = -1;

	return p;
}


void bb_problem_calculator::create_raw_problem3(const raw_bb_problem* father, int indice, int begin_end, raw_bb_problem* child){

	child->begin_end = begin_end;

	limites_set(father, child, begin_end);
	depth_set(father, child);
	permutation_set(father, indice, begin_end, child);
	ranks_set(father, child);
}

void bb_problem_calculator::free_raw_problem(raw_bb_problem* p){
	free (p);
}


void bb_problem_calculator::limites_set(const raw_bb_problem* father, raw_bb_problem* child, int begin_end){
	child->limite1 = father->limite1;
	child->limite2 = father->limite2;

	if (begin_end == BEGIN_ORDER) child->limite1++;
	else	child->limite2--;

	if (child->limite2 - child->limite1 == 2) child->limite1++;
}

bool bb_problem_calculator::simple(const raw_bb_problem* p){
	return (p->depth == this->pbb->size - 2);
}

bool bb_problem_calculator::leaf(const raw_bb_problem* p){
	return (p->depth == this->pbb->size);
}


void bb_problem_calculator::depth_set(const raw_bb_problem* father, raw_bb_problem* child){
	if (simple(father))
		child->depth = pbb->size;
	else
		child->depth = father->depth + 1;
}

void bb_problem_calculator::permutation_set(const raw_bb_problem* father, int indice,int begin_end, raw_bb_problem* child){
	child->job = father->permutation[indice];
	for (int j = 0; j < pbb->size; j++)	child->permutation[j] = father->permutation[j];
	int tmp_indice =	(begin_end == BEGIN_ORDER) ?	father->limite1 + 1 : father->limite2 - 1;

	int tmp = child->permutation[tmp_indice];
	child->permutation[tmp_indice] = child->permutation[indice];
	child->permutation[indice] = tmp;
}

void bb_problem_calculator::ranks_set (const raw_bb_problem* father, raw_bb_problem* child){
	for (int d = 0; d < pbb->size + 1; d++)
			child->ranks[d] = father->ranks[d];
}

void bb_problem_calculator::ranks_depth(raw_bb_problem* child, int rank){
	child->ranks[child->depth] = rank;
}

void bb_problem_calculator::ranks_depth2(raw_bb_problem* child, int rank){
	child->ranks[child->depth-1] = rank;
}

void bb_problem_calculator::print(raw_bb_problem* p)
{
	std::cout << p->couts_somme << " ";

	for (int j = 0; j < pbb->size; j++)	std::cout << p->permutation[j] << " ";

	std::cout << p->limite1 << " " << p->limite2 << " ";

	std::cout << p->depth << " " << p->job << " " << p->begin_end << endl;
}


istream& operator>>(istream& stream, raw_bb_problem& p)
{
	for (int i = 0; i < 2; i++)	stream >> p.couts[i];

	stream >> p.couts_somme;

	for (int j = 0; j < MAX_NBJOBS; j++)	stream >> p.permutation[j];

	stream >> p.limite1 >> p.limite2;

	stream >> p.depth;

	stream >> p.job;

	stream >> p.begin_end;

	return stream;
}

ostream& operator<<(ostream& stream, raw_bb_problem& p)
{

	for (int i = 0; i < 2; i++){
		stream << p.couts[i] << " ";
	}

	stream << p.couts_somme << " ";

	for (int j = 0; j < MAX_NBJOBS; j++)	{
		stream << p.permutation[j] << " ";
	}

	stream << p.limite1 << " " << p.limite2 <<" ";

	stream << p.depth << " ";

	stream << p.job << " ";

	stream << p.begin_end << " ";

	stream << endl;

	return stream;
}

