using namespace std;

#ifndef SELECT_WORST_H
#define SELECT_WORST_H

class select_worst : public select_abstract
{
	public:
	bool operator()(raw_bb_problem const* n1, raw_bb_problem const* n2) const;
};

#endif



