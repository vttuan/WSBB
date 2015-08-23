using namespace std;
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <semaphore.h>
#include <string>

#ifndef PEER_H
#define PEER_H

#define WORKER 2
#define INDEX 3
#define EMPTY 5

class works; class problem; class pbab;  
class peer
{public:
 pbab*pbb;
 string adress, radress; 
 int port, rport; 
 //float power, exploitation; time_t heartbeat;
 peer();
 peer(pbab*pbb);
 peer(string padress, int pport);
 peer(const peer& pr);
 bool is_null();
 void set_null();
 void set_relay(string radress, int rport);
 void set_heartbeat();   
 bool fault();
 void operator=(peer& p);
 bool operator==(peer& p);
 bool operator!=(peer& p);
};

ostream& operator<<(ostream& stream, peer& p);
istream& operator>>(istream& stream, peer& p);
bool operator>(peer p,peer q);
bool operator<(peer p,peer q);

#endif

