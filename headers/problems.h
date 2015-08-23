using namespace std;
#include <vector>
#include <iostream>
#include "../headers/select_abstract.h"
#include "../headers/select_worst.h"

#ifndef problemS_H
#define problemS_H
#include "../headers/bb_problem_calculator.h"

class select_worst;

class problems: public vector<raw_bb_problem*> {
public:
	select_worst select2;
	bb_problem_calculator* bbCalculator;

	int sum(int best);
	void ranks();
	void ranks2();
	int coupe(int best);
	void empty();

	int size1;
	bool push_back1(raw_bb_problem*p, int max);

	void operator>>(ostream& stream);
};
#endif

