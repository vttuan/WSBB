using namespace std;
#include <iostream>
#include "../headers/peer.h"
#include "../headers/ttime.h"
#include "../headers/pbab.h"

peer::peer()
{
	set_null();
}

peer::peer(pbab* pbb)
{
	set_null();
	this->pbb=pbb;
}

peer::peer(string adress, int port)
{
	this->adress=adress;
	this->port=port;
}

peer::peer(const peer& pr)
{
	adress=pr.adress;
	port=pr.port;
	pbb=pr.pbb;
}


bool peer::is_null()
{
	return ((adress=="null")&&(port==0));
}

void peer::set_null()
{
	adress="null";
	port=0;
}

void peer::set_heartbeat()
{//heartbeat=ttime::time_get();
}

void peer::operator=(peer& p)
{
	adress=p.adress;
	port=p.port;
}

bool peer::operator==(peer& p)
{
	return ((adress==p.adress)&&(port==p.port));
}

bool peer::operator!=(peer& p)
{
	return !(*this==p);
}

bool operator<(peer p,peer q)
{
	if (p.adress==q.adress) return p.port<q.port;
	else
	return p.adress<q.adress;
}

bool operator>(peer p,peer q)
{
  if (p.adress==q.adress) return p.port>q.port;
  else return p.adress>q.adress;
}

ostream& operator<<(ostream& stream, peer& p)
{
	stream<<p.adress<<" "<<p.port<<" ";
	return stream;
}

istream& operator>>(istream& stream, peer& p)
{
	stream>>p.adress>>p.port;
	return stream;
}
