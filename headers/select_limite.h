using namespace std;

#ifndef SELECT_LIMITE_H
#define SELECT_LIMITE_H

class select_limite : public select_abstract
{
	public:
	bool operator()(raw_bb_problem const* n1, raw_bb_problem const* n2) const;
};

#endif



