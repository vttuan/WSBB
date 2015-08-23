using namespace std;

#include "types.h"

#ifndef PEER_H
#define PEER_H

//class neighbors;

//class works; class problem; class pbab;  
class manager {
private:
	void error(const char *msg);
	int initialize_server(int port);

	int send_message(packet_t *p);
	void handle_join_request_GPU(packet_t * p);
	void handle_join_request_CPU(packet_t * p);
	void print_peers_data();

public:

};

#endif

