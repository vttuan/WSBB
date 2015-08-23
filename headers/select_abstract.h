#ifndef SELECT_ABSTRACT_H
#define SELECT_ABSTRACT_H
struct raw_bb_problem;
class select_abstract {
public:
	virtual bool operator()(raw_bb_problem const* n1, raw_bb_problem const* n2) const =0;
};
extern select_abstract* slct;

#endif

