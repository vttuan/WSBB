//===============================================================================================
using namespace std;
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <string.h>
#include <list>
#include <map>
#include <time.h>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/peer.h"

#ifndef WORKS_H
#define WORKS_H
class work;
class pbab;

struct biginteger_compare // Du + petit au + grand.
{
	bool operator()(BigInteger b1, BigInteger b2) const
	{
		return (b1 > b2);
	}
};

struct timet_compare // Du + petit au + grand.
{
	bool operator()(time_t t1, time_t t2) const
	{
		return (t1 < t2);
	}
};

struct id_compare // Du + petit au + grand.
{
	bool operator()(unsigned long int t1, unsigned long int t2) const
	{
		return (t1 < t2);
	}
};

typedef map<BigInteger, work*, biginteger_compare> biginteger_type;
typedef multimap<BigInteger, work*, biginteger_compare> sizes_type;
typedef multimap<time_t, work*, timet_compare> times_type;
typedef map<unsigned long int, work*, id_compare> id_type;

typedef biginteger_type::iterator biginteger_iterator;
typedef sizes_type::iterator sizes_iterator;
typedef times_type::iterator times_iterator;
typedef id_type::iterator id_iterator;

class works
{
public:
	pbab*pbb;
	string directory;
	//quelques statistiques
	BigInteger size;
	BigInteger size_prolems, size_foster, size_pico;

	int panne;
	int nouveau;
	int actif;

	//ensemble d'intervalles organisés selon
	sizes_type sizes; // la taille
	times_type times; // le mis à jour
	id_type ids; 	  // l'identité

	bool TIME;

	// la solution optimale ainsi que son cout
	int cout;
	problem*pr;

	works();
	works(string directory, pbab*pbb);
	void init();
	void save();
	void clear();

	//gerer l'ensemble id
	void id_insert(work* w);
	void id_delete(work* w);
	void id_update(work* w);
	work*id_find(work* w); // retrouver un work d'un certain id s'il existe 
			       // (je devrais passer que l'id ici)

	//gerer l'ensemble times
	void times_insert(work* w, bool fault = false);
	void times_delete(work* w);
	void times_update(work* w);
	work*times_fault(); 	// retrouver un intervalle dont le processus traitant est en panne
	work*times_oldest(); 	// retrouv le plus vieux intervale

	//gerer l'ensemble sizes
	void sizes_insert(work* w);
	void sizes_delete(work* w);
	void sizes_update(work* w);
	work*sizes_big(); //retrouver le plus grand intervalle

	//gerer les intervalles
	work*_update(work* w);	// mettre a jour un intervalle
	work*_fault(); 		// recuperer un intervalle dans le processus est en panne
	work*_big(bool divise); //recuperer la seconde moitie d'un intervalle
	work*_oldest(); 	// recuperer le plus ancen intervalle

	void request(work& w);

	void request(work& w, problem*_pr);
};

ostream& operator<<(ostream& stream, works& ws);
istream& operator>>(istream& stream, works& ws);

#endif

