//===============================================================================================
#include <limits.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <string.h>
#include <list>
#include <map>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/work.h"
#include "../headers/ttime.h"
#include "../headers/weights.h"
#include "../headers/pbab.h"
#include "../headers/works.h"
#include "../headers/problem.h"
#include "../headers/tree.h"
using namespace std;

//===============================================================================================

works::works()
{
	panne = nouveau = actif = 0;
	cout = INT_MAX;
	pr = NULL;
}

works::works(string directory, pbab*pbb)
{
	this->pbb = pbb;
	this->directory = directory;
	//initiliser l'intervalle (voir fonction init())
	//initialiser les statistiques
	panne = nouveau = actif = 0;
	// initialiser la solution
	cout = INT_MAX;
	pr = new problem(pbb);
}

void works::init()
{
	size = 0; //size_prolems=0; size_foster=0;  size_pico=0;
	ifstream stream((directory + "data/bab.works").c_str());

	if (stream != NULL)
	{
		stream >> *this;
		stream.close();
	}
	else
	{
		work*w = new work(0, pbb->wghts->depths[0], pbb);
		id_insert(w);
		sizes_insert(w);
		times_insert(w, true);
	}
}

void works::save()
{
	BigInteger zero = 0;

	if (pbb->ttm->period_passed(CHECKPOINT_TTIME) || (size == zero)) /*continue*/	;
	else	return;

	ofstream stream((directory + "bab.works").c_str());
	stream << *this;
	stream.close();
}

void works::clear()
{
	size = 0;
	for (id_iterator i = ids.begin(); i != ids.end(); ++i)		work_free(i->second);
	ids.clear();
	sizes.clear();
	times.clear();
}

//===============================================================================================

void works::sizes_insert(work* w)
{
	w->set_size();
	size += w->size;
	sizes.insert(sizes_type::value_type(w->size, w));
}

void works::sizes_delete(work* w)
{
	pair<sizes_iterator, sizes_iterator> range = sizes.equal_range(w->size);

	for (sizes_iterator i = range.first; i != range.second; i++)
		if (i->second == w)
		{
			size -= w->size;
			sizes.erase(i);
			break;
		}
}

void works::sizes_update(work* w)
{
	sizes_delete(w);
	sizes_insert(w);
}

//===============================================================================================

void works::id_insert(work* w)
{
	ids.insert(sizes_type::value_type(w->id, w));
}

void works::id_delete(work* w)
{
	id_iterator i = ids.find(w->id);
	if (i != ids.end())
		ids.erase(i);
}

void works::id_update(work* w)
{
	id_delete(w);
	w->set_id();
	id_insert(w);
}

//===============================================================================================

void works::times_insert(work* w, bool fault)
{
	if (fault)	w->time = 0;
	else	w->set_time();

	times.insert(times_type::value_type(w->time, w));
}

void works::times_delete(work* w)
{
	pair<times_iterator, times_iterator> range = times.equal_range(w->time);

	for (times_iterator i = range.first; i != range.second; i++)
		if (i->second->id == w->id)
		{
			times.erase(i);
			break;
		}
}

void works::times_update(work* w)
{
	times_delete(w);
	times_insert(w);
}

//===============================================================================================

work*works::id_find(work* w)
{
	id_iterator tmp = ids.find(w->id);
	return (tmp == ids.end()) ? NULL : tmp->second;
}

work*works::times_fault()
{
	work*w = times.begin()->second;
	return (w->fault()) ? w : NULL;
}

work*works::sizes_big()
{
	work*w = sizes.begin()->second;
	return (w->big()) ? w : NULL;
}

work*works::times_oldest()
{
	return times.begin()->second;
}

//===============================================================================================
bool fault;

work* works::_update(work* w)
{
	work*tmp = id_find(w);
	fault = (tmp != NULL);

	if (tmp == NULL)	return NULL;

	tmp->intersection(w);

	if (tmp->empty())
	{
		times_delete(tmp);
		sizes_delete(tmp);
		id_delete(tmp);
		work_free(tmp);
		return NULL;
	}

	times_update(tmp);
	sizes_update(tmp);
	return tmp;
}

work*works::_fault()
{
	work*tmp = times_fault();

	if (tmp == NULL)	return NULL;

	times_update(tmp);
	return tmp;
}

work*works::_big(bool divise)
{
	work*tmp1 = sizes_big();
	if ((tmp1 == NULL) || (!divise))
		return tmp1;
	work*tmp2 = tmp1->divide();
	sizes_update(tmp1);
	sizes_insert(tmp2);
	id_insert(tmp2);
	times_insert(tmp2);
	return tmp2;
}

work*works::_oldest()
{
	work*tmp = times_oldest();
	times_update(tmp);
	return tmp;
}

//===============================================================================================

void works::request(work& w)
{
	work* tmp = _update(&w);

	if (!ids.size())	w.set_finished();
	else
	{
		if (tmp == NULL)
		{
			tmp = _fault();
			if (!tmp)	panne++;
		}

		if (tmp == NULL)
		{
			tmp = _big(true);
			if (!tmp)	nouveau++;
		}

		if (tmp == NULL)
			tmp = _oldest();

		w = *tmp;
	}
}

void works::request(work& w, problem*_pr)
{
	actif++;

	if((_pr) )	*pr=*_pr;

	request(w);
}

//===============================================================================================

ostream& operator<<(ostream& stream, works& ws)
{
	stream << ws.ids.size() << endl;

	for (id_iterator i = ws.ids.begin(); i != ws.ids.end(); ++i)	stream << *(i->second);

	return stream;
}

istream& operator>>(istream& stream, works& ws)
{
	ws.clear();
	int number;
	stream >> number;

	for (int i = 0; i < number; i++)
	{
		work*w = new work(ws.pbb, stream);
		w->pr.set_null(); // à la lecture, tous les peers sont en panne ...
		ws.sizes_insert(w);
		ws.id_insert(w);
		ws.times_insert(w, true);
	}

	return stream;
}

