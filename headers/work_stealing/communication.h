using namespace std;
#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


#include "types.h"
#include "packets.h"

class Node;

#include <pthread.h>

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

//class neighbors;

//class works; class problem; class pbab;  
class communication
{
	public:
		uint64_t PORT;
		Node * ndx;
		long int packets_received;
		packets * pcks;

		communication(Node *nd);
		void error(const char *msg);
		void initialize_my_address(address_t * address);
		int make_socket (uint16_t port);
		void init_sockaddr (struct sockaddr_in *name, char *hostname, uint16_t port);
		void receive_remote_msg(void * filedes);
		int send_message(packet_t *p);

};

#endif

