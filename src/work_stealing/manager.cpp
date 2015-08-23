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
#include <pthread.h>
#include <errno.h>

#include "../../headers/work_stealing/manager.h"

#define PORT 52000

pthread_mutex_t packet_container_mutex = PTHREAD_MUTEX_INITIALIZER;

int number_of_packets = 0;
packet_container_t * first_container = NULL;
packet_container_t * last_container = NULL;

int add_packet(packet_t *p);
packet_t * get_packet();
int get_number_of_packets();

int add_packet(packet_t *p) {

	pthread_mutex_lock(&packet_container_mutex);

	packet_container_t *pc = (packet_container_t *) malloc(
			sizeof(packet_container_t));
	pc->data_packet = p;

	number_of_packets = number_of_packets + 1;

	if (number_of_packets == 1) {
		first_container = pc;
		last_container = pc;
		pc->next_container = NULL;
		pc->previous_container = NULL;
	}

	if (number_of_packets > 1) {
		pc->previous_container = last_container;
		pc->next_container = NULL;
		last_container->next_container = pc;
		last_container = pc;
	}

	pthread_mutex_unlock(&packet_container_mutex);
	return 0;
}

packet_t * get_packet() {

	pthread_mutex_lock(&packet_container_mutex);
	packet_t *p = NULL;

	if (number_of_packets > 0) {

		p = first_container->data_packet;
		number_of_packets = number_of_packets - 1;

		if (number_of_packets == 0) {
			packet_container_t * tmp = first_container;
			first_container = NULL;
			last_container = NULL;
			free(tmp);

		}

		if (number_of_packets > 0) { //if there exists some more packets

			packet_container_t * tmp = first_container; //assign the first container to tmp
			first_container = tmp->next_container; //now the first container points to next container
			first_container->previous_container = NULL; //as now there does not exists any previous container
			free(tmp); //free the memory for the tmp container

		} //endif(number_of_packets >0)

	} //endif(number_of_packets >0)

	pthread_mutex_unlock(&packet_container_mutex); //release the packet_container lock
	return p;
}
/**************************************************************************/
int get_number_of_packets() {

	return number_of_packets;
}

/************************end Class code of packets_queue************************/

int current_peers = -1; // current number of peers in the tree/network

int number_of_peers = 0;

int current_parent_id = 0;
int nchilds = 5;
int current_parent_nchilds = 0; //fixed number of childs for each node

peer_t * peers_data[max_peers]; // Array of pointers for peers data
peer_t *last_peer = NULL;
peer_t *first_peer = NULL;

int duration = 600; //default running time for the manager

address_t *my_address = (address_t*) malloc(sizeof(address_t));
;
//structure to carry my own address

void handle_join_request(packet_t * p);
/**************************************************************************/
void error(const char *msg) {
	perror(msg);
	exit(0);
}

/**************************************************************************/
void initialize_my_address() {

//char hostname[100];
//hostname[99] = '\0';
//gethostname(hostname, 100);

	gethostname(my_address->host_machine, 100);
	my_address->id = -1;
	my_address->listening_port = PORT;

	return;
}
/**************************************************************************/
//-------------------------make_socket() used in bootstrap-------------------------------------------
int make_socket(uint16_t port) {

	int sock;

	struct sockaddr_in name;
// Create the socket.
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	linger* set_linger = (linger*) malloc(sizeof(linger));

	int yes = 1;
	set_linger->l_onoff = 1;
	set_linger->l_linger = 60;
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &set_linger,
			sizeof(set_linger));
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

// Give the socket a name.
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &name, sizeof(struct sockaddr_in)) < 0) {
		error("bind");
		exit(EXIT_FAILURE);
	}

	return sock;

}

//--------------------init_sockadd()--called in send_remote_msg---------------------------------

void init_sockaddr(struct sockaddr_in *name, char *hostname, uint16_t port) {

//pthread_mutex_lock(&sock_initialize_mutex);
	struct hostent *hostinfo;
	name->sin_family = AF_INET;
	name->sin_port = htons(port);

	hostinfo = gethostbyname(hostname);

	if (hostinfo == NULL) {
//fprintf (stderr, "remote_interface.c: Unknown host (init_sockaddr())%s.\n", hostname);
		cout << "remote_interface.c: Unknown host (init_sockaddr()) = "
				<< hostname << endl;
		exit(EXIT_FAILURE);
	}

	name->sin_addr = *(struct in_addr *) hostinfo->h_addr;

}

//-----------------called uopn receiving a message from remote node from remote machine--------------------

void receive_remote_msg(void * filedes) {
	//To separate the pointers from other threads, we attach _rcv to all the pointers in the receive_remote_msg function.
	//pthread_detach(pthread_self());
	//printf("inside receive_remote_message message received.\n");

	//creates a new remote_packet on local machine
	//int packet_added=0;

	packet_t *packet_rcv = (packet_t *) malloc(sizeof(packet_t));

	if (packet_rcv == NULL) {
		perror(
				"remote_interface.c: error in malloc packet_rcv (receive_remote_msg())\n");
		exit(EXIT_FAILURE);
	};

	char * buffer_rcv;
	if ((buffer_rcv = (char *) malloc(sizeof(packet_t))) == NULL) {
		perror(
				"remote_interface.c: error in malloc buffer_rcv (receive_remote_msg())\n");
		exit(EXIT_FAILURE);
	};

	size_t nbytes;
	size_t mbytes;
	int connfd;

	connfd = *((int *) filedes);

	nbytes = read(connfd, buffer_rcv, sizeof(packet_t));
	//printf("read bytes from socket= %d\n", nbytes);
	memcpy(packet_rcv, buffer_rcv, sizeof(packet_t));

	//printf ("packet receive data size = %d\n", packet_rcv -> size);
	//cout<<"manager: packet cved from machine # "<<packet_rcv->host_machine<<" peer # "<<packet_rcv->host_peer<<" msg_type = "<<packet_rcv->message_type<<endl;

	packet_rcv->data = (char *) malloc(packet_rcv->size);
	if (packet_rcv->data == NULL) {
		perror(
				"peer.c: error in malloc packet_rcv->data (receive_remote_msg())\n");
		exit(EXIT_FAILURE);
	};

	mbytes = read(connfd, packet_rcv->data, packet_rcv->size);

	//printf("read bytes from socket = %d\n", nbytes);

	if (nbytes < sizeof(packet_t)) {
		//Read error.
		perror("manager.cpp: read socket error (receive_remote_msg())\n");
		exit(EXIT_FAILURE);
	} //end if
	else if (nbytes == 0)

	{
		printf(
				"remote_interface.c: received nbytes zero (receive_remote_msg()).");
	} //end else if

	else {
		free(buffer_rcv);
		if (filedes) {
			free(filedes);

		} //end if

		//cout<<"manager: packet rcved from machine # "<<packet_rcv->host_machine<<" peer # "<<packet_rcv->host_peer<<" msg_type = "<<packet_rcv->message_type<<endl;

		/*
		 //free received packet
		 if(packet_rcv!=NULL){
		 if(packet_rcv -> packet_type == 1){
		 if(packet_rcv -> data!=NULL){ free(packet_rcv -> data);}
		 }
		 free(packet_rcv);}

		 //printf("remote packet added to receive channel.\n");
		 */
	} //end else

	close(connfd);

	add_packet(packet_rcv);

	/*
	 if(packet_rcv ->message_type==0){
	 handle_join_request(packet_rcv);
	 }
	 */

	return;
	//pthread_exit((void *) 0);
	/*
	 fprintf(stderr, "received remote messgae from source, seq = %d :  %d.\n", remote_packet -> source_id_local, remote_packet -> message_seq);
	 */
	//return 0;
}

/**************************************************************************/
//---------------------------called to bootstrap---------------------------
void* manager_bootstrap(void*) {

	int servsock_boot, *new_boot;
	struct sockaddr_in clientname_boot;
	size_t size;

	//Create the socket and set it to accept connections.

	servsock_boot = make_socket(PORT);
	if (listen(servsock_boot, 100) < 0) {
		error(
				"manager_bootstrap.cpp: error in listen socket (peer_bootstrap())");
		exit(EXIT_FAILURE);
	}

	cout << "manager created: listening on port = " << PORT<<endl;
	;

	while (1) {
		//cout<<"packet received. calling receive_remote_msg()"<<endl;
		if ((new_boot = (int *) malloc(sizeof(int))) == NULL) {
			error(
					"manager.cpp: malloc() error new_boot (manager_bootstrap())\n");
			exit(EXIT_FAILURE);
		}
		//size = sizeof(clientname);
		//size= sizeof(struct sockaddr_in);
		size = sizeof(struct sockaddr);
		*new_boot = accept(servsock_boot, (struct sockaddr *) &clientname_boot,
				(socklen_t *) &size);

		if (*new_boot < 0) {
			error("manager.cpp: error in accept (manager_bootstrap())\n");
			exit(EXIT_FAILURE);
		}
		//cout<<"packet received. calling receive_remote_msg()"<<endl;
		receive_remote_msg(new_boot);
		/*
		 pthread_t thread_id_boot;
		 //printf("message received now goingt to call receive_remote_msg inside remote_interface_bootstrap.\n");

		 if (pthread_create(&thread_id_boot, NULL, (void *)receive_remote_msg, new_boot) != 0){

		 perror("remote_interface.c: error in thread creation (remote_interface_bootstrap())\n");
		 exit(EXIT_FAILURE);
		 }

		 */
		/*fprintf (stderr, "Server: connect from host %s, port %hd.\n", inet_ntoa (clientname.sin_addr),
		 ntohs (clientname.sin_port));
		 */

	} //end while()

//return 0;
	return NULL;
}

/**************************************************************************/
void* processing_bootstrap(void*) {

	while (1) {
		packet_t *p = get_packet();

		if (p != NULL) {
			if (p->message_type == 0) {
				cout << "Manager: received a join request from machine = "
						<< p->host_machine << " and port = " << p->host_port
						<< endl;
				handle_join_request(p);
			}
		}

		if (p != NULL) {
			if (p->size > 0) {
				if (p->data != NULL) {
					free(p->data);
				}
			}
			free(p);
		}

	} //end while()

	return NULL;
}

/**************************************************************************/
int send_message(packet_t *p) {

	cout << "Manager: sending message type " << p->message_type << " to node "
			<< p->dest_peer << " on machine = " << p->dest_machine
			<< " on port = " << p->dest_port << endl;
	char *buffer = NULL;
	int total_packet_size = 0;

	total_packet_size = (sizeof(packet_t) + (p->size));
	//printf("total_packet_size = %d\n", total_packet_size);

	buffer = (char *) malloc(total_packet_size);

	if (buffer == NULL) {
		perror("peer.cpp: error in malloc() buffer (send_packet())\n");
		exit(EXIT_FAILURE);
	};

	if (buffer != NULL) {

		memcpy(buffer, p, sizeof(packet_t));
		memcpy((buffer + sizeof(packet_t)), p->data, p->size);
	} //end if (buffer!= NULL)

	else {
		perror("peer.cpp: error in malloc() buffer (send_packet())\n");
		exit(EXIT_FAILURE);
	} //end else


	//cout<<"inside send message"<<endl;
	char* server_name = p->dest_machine;
	int server_port = p->dest_port;

	int sock;
	struct sockaddr_in server;

	//sock= socket(AF_INET, SOCK_DGRAM, 0);
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		error("socket");

	init_sockaddr(&server, server_name, server_port);

	//cout<<"manager: connecting to = "<<server_name<<" port = "<<server_port<<endl;
	if (0 > connect(sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("manager.cpp: error connect (client) (send_packet())\n");
		exit(EXIT_FAILURE);
	} //endif

	int nbytes = write(sock, buffer, total_packet_size);
	//printf("written bytes on socket = %d\n", nbytes);

	if (nbytes < total_packet_size) {
		perror("manager.cpp: write socket (send_packet())\n");
		exit(EXIT_FAILURE);
	} //end if

	//cout<<"peer # "<<p->host_peer<<" msg type = "<<p->message_type<<" sent to machine = "<<p->dest_machine<<" port # "<<p->dest_port<<endl;

	if (p != NULL) {
		if (p->size > 0) {
			if (p->data != NULL) {
				free(p->data);
			}
		}
		free(p);
	}

	close(sock); // close connection socket

	//free(p); //free packet memory

	return 0;
}

/**************************************************************************/

int do_parse(int argc, char *argv[]) {
	char c;

	while ((c = getopt(argc, argv, "d:n:c:m")) != -1) {
		switch (c) {

		case 'd':
			duration = atoi(optarg);
			break;

		case 'n':
			number_of_peers = atoi(optarg);
			break;

		case 'c':
			nchilds = atoi(optarg);
			break;

		}
	}

	return 0;
}

/**************************************************************************/
address_t * get_address() {

	return my_address;
}

/**************************************************************************/
void print_address() {

	cout << "peer_id = " << my_address->id << " host_machine = "
			<< my_address->host_machine << " listening_port = "
			<< my_address->listening_port << endl;

	return;
}

/**************************************************************************/
void print_peers_data() {
	int total_number_of_peers = current_peers + 1;
	cout << "Total number of peers = " << total_number_of_peers << endl;
	peer_t * p;
	int total_height = 0;
	int max_height = 0;
	int min_height = 100000;
	int total_degree = 0;
	int max_degree = 0;
	int min_degree = 100000; //any arbitrart value
	int average_degree = 0;
	int average_height = 0;

	for (int i = 0; i <= current_peers; i++) {
		p = peers_data[i];
		int tmp_degree = p->number_of_childs + 1;
		total_degree = total_degree + tmp_degree;
		total_height = total_height + p->height;
		if (tmp_degree > max_degree) {
			max_degree = tmp_degree;
		}
		if (tmp_degree < min_degree) {
			min_degree = tmp_degree;
		}
		if (p->height > max_height) {
			max_height = p->height;
		}
		if (p->height < min_height) {
			min_height = p->height;
		}

		cout << "Id=" << p->id << " Parent=" << p->parent_id << " height= "
				<< p->height << " no_childs=" << p->number_of_childs
				<< " child_ids= ";
//print child ids
		for (int j = 0; j < p->number_of_childs; j++) {
			cout << p->childs[j] << " ";
		}
		cout << endl;
	}
	cout << "total_height = " << total_height << " total_degree = "
			<< total_degree << endl;
	average_height = total_height / total_number_of_peers;
	average_degree = total_degree / total_number_of_peers;

	cout << "max_height = " << max_height << " min_height = " << min_height
			<< " average_height = " << average_height << " max_degree = "
			<< max_degree << " min_degree = " << min_degree
			<< " average_degree = " << average_degree << endl;

	return;
}
/**************************************************************************/
int send_trigger_signal() {

	packet_t *p = (packet_t *) malloc(sizeof(packet_t));

	p->message_type = 11; //reply to a joining request
	p->size = 0; //packet constains no data

	//addrerss information of the destination peer
	p->dest_peer = first_peer->id; //id of the calling peer
	memcpy(p->dest_machine, first_peer->host_machine, 100); //machine of the sending peer
	p->dest_port = first_peer->listening_port; //listening port of the sending peer

	//address information of the sending peer(manager)
	p->host_peer = my_address->id;
	memcpy(p->host_machine, my_address->host_machine, 100);
	p->host_port = my_address->listening_port;

	//send packet
	send_message(p);

	return 0;
}

int broadcast_trigger_signal() {

	for (int i = 0; i < number_of_peers; i++) {
		packet_t *p = (packet_t *) malloc(sizeof(packet_t));

		p->message_type = 11;
		p->size = 0; //packet constains no data

		//addrerss information of the destination peer
		p->dest_peer = peers_data[i]->id; //id of the calling peer
		memcpy(p->dest_machine, peers_data[i]->host_machine, 100); //machine of the sending peer
		p->dest_port = peers_data[i]->listening_port; //listening port of the sending peer

		//address information of the sending peer(manager)
		p->host_peer = my_address->id;
		memcpy(p->host_machine, my_address->host_machine, 100);
		p->host_port = my_address->listening_port;

		//send packet
		send_message(p);
	}
	return 0;
}

void broadcast_ip_of_all_networks() {

	cout << "Broadcasting IP to " << number_of_peers << endl;

	// peer id, listening port and host address
	int size = (sizeof(peerid_t) + sizeof(uint64_t) + 100) * number_of_peers;

	char *data_needs_to_be_sent = (char*) malloc(size);
	int index = 0;

	for (int i = 0; i < number_of_peers; i++) {
		memcpy((data_needs_to_be_sent + index), &(peers_data[i]->id),
				sizeof(peerid_t));
		index = index + sizeof(peerid_t);

		memcpy((data_needs_to_be_sent + index),
				&(peers_data[i]->listening_port), sizeof(uint64_t));
		index = index + sizeof(uint64_t);

		memcpy((data_needs_to_be_sent + index), peers_data[i]->host_machine,
				100);
		index = index + 100;
	}

	for (int i = 0; i < number_of_peers; i++) {

		packet_t *data_packet = (packet_t *) malloc(sizeof(packet_t));

		data_packet->message_type = 14; // broadcast ip

		//addrerss information of the destination peer
		data_packet->dest_peer = peers_data[i]->id;
		memcpy(data_packet->dest_machine, peers_data[i]->host_machine, 100);
		data_packet->dest_port = peers_data[i]->listening_port;

		//address information of the sending peer(manager)
		data_packet->host_peer = my_address->id;
		memcpy(data_packet->host_machine, my_address->host_machine, 100);
		data_packet->host_port = my_address->listening_port;

		data_packet->size = size;
		data_packet->data = (char*) malloc(size);

		memcpy(data_packet->data, data_needs_to_be_sent, data_packet->size);

		send_message(data_packet);
	}

	free(data_needs_to_be_sent);
}

/**************************************************************************/
void handle_join_request(packet_t * p) {

	current_peers = current_peers + 1; //increase the number of current peers

	cout << "TUAN, peer = " << p->host_machine << endl;
	cout << "TUAN, port = " << p->host_port << endl;

	peer_t *new_peer = (peer_t *) malloc(sizeof(peer_t)); //create space for a new peer

	// Add the peer to the peers
	if (p->message_type == 0) {
		// initializes the new_peers data
		new_peer->id = current_peers;
		memcpy(new_peer->host_machine, p->host_machine, 100);
		new_peer->listening_port = p->host_port;
		new_peer->number_of_childs = 0;

		// if it is a first peer
		if (current_peers == 0) {
			first_peer = new_peer;
			last_peer = new_peer;
			new_peer->previous_peer = NULL;
			new_peer->next_peer = NULL;

			//it is root so it will be its own peer
			new_peer->parent_id = 0;
			new_peer->height = 0;
			current_parent_id = 0; //we initialize the current parent id to zero
			current_parent_nchilds = 0;

		} //endif(current_peers==0)

		// if it is not a first peer
		if (current_peers > 0) {
			new_peer->next_peer = NULL;
			new_peer->previous_peer = last_peer; //connects it to the previous peer which is last_peer
			last_peer->next_peer = new_peer; //connects the previous peer to this new_peer
			last_peer = new_peer; //updates the last_peer pointer to point to the current child

			if (current_parent_nchilds < nchilds) {
				new_peer->parent_id = current_parent_id;
				current_parent_nchilds = current_parent_nchilds + 1;
			} else {
				current_parent_id = current_parent_id + 1;
				current_parent_nchilds = 0;

				new_peer->parent_id = current_parent_id;
				current_parent_nchilds = current_parent_nchilds + 1;
			}

			//find a random parent which has childs less than max childs allowed
			//	random_id=rand() % current_peers;
			//cout<<"random_id="<<random_id<<endl;
			//	while(peers_data[random_id]->number_of_childs >= max_childs){
			//	random_id=rand() % current_peers;
			//	}
			//	new_peer->parent_id=random_id;

		} //endif(current_peers>0)

		//we add the child always in the last
		new_peer->next_peer = NULL; /*As we always add the peer at the last so the next_sibling pointer of the last child will always be NULL*/

	} //end if (p->message_type==0)

	peers_data[current_peers] = new_peer; //adds the new_peer to the peers data

	//updates the parents childs
	if (new_peer->parent_id != new_peer->id) { //If it is not root
		peers_data[new_peer->parent_id]->childs[peers_data[new_peer->parent_id]->number_of_childs] =
				new_peer->id;
		new_peer->height = peers_data[new_peer->parent_id]->height + 1;

		//increment the parents number of childs
		peers_data[new_peer->parent_id]->number_of_childs =
				peers_data[new_peer->parent_id]->number_of_childs + 1;
	}

	//send the peer_id and parent_id and address of the parent back to joining peer.
	//prepare the packet to send back to joining node
	packet_t *data_packet = (packet_t *) malloc(sizeof(packet_t));

	data_packet->message_type = 1; 	//reply to a joining request
	data_packet->size = 0; 			//packet constains no data

	//addrerss information of the destination peer
	data_packet->dest_peer = new_peer->id; 							//id of the calling peer
	memcpy(data_packet->dest_machine, new_peer->host_machine, 100); //machine of the sending peer
	data_packet->dest_port = new_peer->listening_port; 				//listening port of the sending peer

	//address information of the sending peer(manager)
	data_packet->host_peer = my_address->id;
	memcpy(data_packet->host_machine, my_address->host_machine, 100);
	data_packet->host_port = my_address->listening_port;

	//parent id and address information to be sent to peer node
	data_packet->data_id = new_peer->parent_id;
	memcpy(data_packet->data_machine,
			peers_data[new_peer->parent_id]->host_machine, 100);
	data_packet->data_port = peers_data[new_peer->parent_id]->listening_port;

	//send packet
	send_message(data_packet);

	cout << "current_peers = " << current_peers + 1 << endl;

	if (current_peers == (number_of_peers - 1)) {

		//we sleep for 15 seconds just to make sure construction of the network is completed
		sleep(15);
		broadcast_ip_of_all_networks();

		sleep(10);
//		send_trigger_signal();
		broadcast_trigger_signal();

		print_peers_data();
		cout << "Network completed. Manager Exiting." << endl;
		exit(0);
	}

	return;
}

/**************************************************************************/
int main(int argc, char**argv) {

	srand(time(NULL)); // initialize the random number

	do_parse(argc, argv);
	initialize_my_address();

	print_address();

	cout << "Network Size = " << number_of_peers << endl;
	cout << "Number of childs of a node = " << nchilds << endl;

	//creating listening thread
	pthread_t listening_thread;
	int listening_thread_id;
	listening_thread_id = pthread_create(&listening_thread, NULL,
			manager_bootstrap, NULL);

	//creating processing thread
	pthread_t processing_thread;
	int processing_thread_id;
	processing_thread_id = pthread_create(&processing_thread, NULL,
			processing_bootstrap, NULL);

	sleep(duration);

	print_peers_data();

	cout << "Specified running time over = " << duration << " seconds" << endl
			<< "Main Terminating" << endl;

	return 0;
}
