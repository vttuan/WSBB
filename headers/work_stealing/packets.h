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

#include <pthread.h>

#ifndef PACKETS_H
#define PACKETS_H

class packets {
public:
	int number_of_packets;
	packet_container_t * first_container;
	packet_container_t * last_container;

	packets();

	int add_packet(packet_t *p);
	packet_t * get_packet();
	int get_number_of_packets();

};

#endif

