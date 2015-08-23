//===============================================================================================
using namespace std;
#include <pthread.h>
#include "../headers/peer.h"
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include <iostream>

#ifndef WORK_H
#define WORK_H
class peer;
class pbab;
class work {
public:
	peer pr;
	pbab*pbb;
	BigInteger begin, end, size;
	BigInteger size_prolems, size_foster, size_pico;
	int id;
	time_t time; // Dernière date de mise à jour
		     // utilisé par le coordinateur et le processus B&B
	//Constructeur
	work(pbab*pbb);
	work(const work& w);
	work(pbab*pbb, istream& stream);
	work(BigInteger b, BigInteger e, pbab*pbb = NULL);

	//work operators
	void unionn(work* w);
	void intersection(work* w);
	work* subtraction(work* w);
	work* divide();

	//Gestion des variables membres
	void set_size();
	void set_time();
	void set_peer(peer& p);
	void set_finished();
	void set_null();
	void set_id();

	//checking
	bool big();
	bool big2();
	bool finished();
	bool empty();
	bool fault();
	bool update();

	bool disjoint(work* w);
	bool contain(work* w);

	//operators
	void operator=(work& w);
	bool equal(work& w);
	bool differ(work& w);
};

ostream& operator<<(ostream& stream, work& w);
istream& operator>>(istream& stream, work& w);
void work_free(work*);
#endif

