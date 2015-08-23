using namespace std;
#include <pthread.h>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/ttime.h"
#include "../headers/weights.h"
#include "../headers/peer.h"
#include "../headers/instance_abstract.h"
#include "../headers/pbab.h"
#include "../headers/work.h"

work::work(const work& w)
{
	pbb = w.pbb;
	begin = w.begin;
	end = w.end;
	size = w.size;
	time = w.time;
	id = w.id;
}

work::work(BigInteger b, BigInteger e, pbab*pbb)
{
	begin = b;
	end = e;
	size = 0;
	time = 0;
	this->pbb = pbb;
	set_id();
}

work::work(pbab*pbb)
{
	begin = 0;
	size = 0;
	this->pbb = pbb;
	id = -1;
}

work::work(pbab*pbb, istream& stream)
{
	stream >> *this;
}

void work::set_null()
{
	begin = 0;
	end = 0;
}

void work::set_size()
{
	if (begin < end)
		size = end - begin;
	else
		size = 0;
}

void work::set_time()
{
	time = ttime::time_get();
}

void work::set_finished()
{
	begin = -1;
	end = -1;
}

void work::set_peer(peer& pr)
{
	this->pr = pr;
}

int id_generator = 0;
void work::set_id()
{
	id = (++id_generator);
}

void min(BigInteger& i1, BigInteger& i2)
{
	if (i1 > i2)
		i1 = i2;
}

void max(BigInteger& i1, BigInteger& i2)
{
	if (i1 < i2)
		i1 = i2;
}

void work::unionn(work* w)
{
	min(begin, w->begin);
	max(end, w->end);
}

work* work::subtraction(work* w)
{
	work*empty = new work(*this);
	empty->begin = empty->end;

	bool ok1 = (begin <= w->begin), ok2 = (w->end <= end);
	if (ok1 && ok2)
	{
		end = w->begin;
		empty->begin = w->end;
	}
	else if (ok1)
		min(end, w->begin);
	else if (ok2)
		max(begin, w->end);

	return empty;
}

void work::intersection(work* w)
{
	max(begin, w->begin);
	min(end, w->end);
}

work* work::divide()
{
	BigInteger coupe;
	coupe = begin;
	coupe += end;
	coupe /= 2;
	work* tmp =new work(coupe, end, pbb); //the second half
	end = coupe; 						  //the first half
	return tmp;
}

bool work::equal(work& w)
{
	return ((begin == w.begin) && (end == w.end));
}

bool work::differ(work& w)
{
	return ((begin != w.begin) || (end != w.end));
}

void work::operator=(work& w)
{
	stringstream stream;
	stream << w;
	stream >> *this;
}

bool work::empty()
{
	return (begin >= end);
}

bool work::finished()
{
	BigInteger moinsun = -1;
	return (begin == moinsun);
}

bool work::fault()
{
	return (ttime::time_get())	> (time + 1.2 * pbb->ttm->periods[WORKER_BALANCING]);
}

const BigInteger EPSILON = 100;
bool work::big()
{
	return big2();
}

const BigInteger deux = 2;

bool work::big2()
{
	set_size();
	return (size > deux);
}

bool work::disjoint(work* w)
{
	return ((w->end <= begin) || (end <= w->begin));
}

bool work::contain(work* w)
{
	return ((begin <= w->begin) && (w->end <= end));
}

ostream& operator<<(ostream& stream, work& w)
{
	stream << w.begin << " " << w.end << " " << w.id << endl;
	stream << w.pr << endl;
	return stream;
}

istream& operator>>(istream& stream, work& w)
{
	string b;
	stream >> b;
	w.begin = easyStringToBI(b);
	string e;
	stream >> e;
	w.end = easyStringToBI(e);
	stream >> w.id;
	stream >> w.pr;
	return stream;
}

void work_free(work*w)
{
	delete w;
}

