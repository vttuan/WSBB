using namespace std;

#include <errno.h>
#include <stdlib.h>

#include "../../headers/work_stealing/communication.h"
#include "../../headers/work_stealing/node.h"
#include "../../headers/work_stealing/packets.h"
#include "../../headers/work_stealing/statistics.h"

communication::communication(Node *nd) {
	this->packets_received = 0;
	this->ndx = nd;
	this->PORT = 12345;
	this->pcks = new packets();

}

void communication::error(const char *msg) {
	perror(msg);
	exit(0);
}

void communication::initialize_my_address(address_t * my_address) {

	char hostname[100];
	hostname[99] = '\0';
	gethostname(hostname, 100);

	gethostname(my_address->host_machine, 100);
	my_address->id = -2;
	my_address->listening_port = PORT;

	return;
}

int communication::make_socket(uint16_t port) {
	int sock;

	struct sockaddr_in name;
	// Create the socket.
	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		perror("Create socket");
		exit(EXIT_FAILURE);
	}

	linger* set_linger = (linger*) malloc(sizeof(linger));

	// Give the socket a name.
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *) &name, sizeof(struct sockaddr_in)) < 0) {
		this->error("bind");
		exit(EXIT_FAILURE);
	}
	return sock;
}

void communication::init_sockaddr(struct sockaddr_in *name, char *hostname,
		uint16_t port) {

	//pthread_mutex_lock(&sock_initialize_mutex);
	struct hostent *hostinfo;
	name->sin_family = AF_INET;
	name->sin_port = htons(port);

	hostinfo = gethostbyname(hostname);
	int attempt = 0;

	while (hostinfo == NULL && attempt < 10) {
		switch (h_errno) {
		case HOST_NOT_FOUND:
			cout << "ERROR: The specified host is unknown" << endl;
			break;
		case NO_ADDRESS:
			cout
					<< "ERROR: The requested name is valid but does not have an IP address: NO_ADDRESS"
					<< endl;
			break;
		case NO_RECOVERY:
			cout << "ERROR: A non-recoverable name server error occurred."
					<< endl;
			break;
		case TRY_AGAIN:
			cout
					<< "ERROR: A temporary error occurred on an authoritative name server.  Try again later."
					<< endl;
			break;
		}
		sleep(1);
		attempt++;
		hostinfo = gethostbyname(hostname);
	}

	if (hostinfo == NULL) {
		cout
				<< "communication::init_sockaddr: Unknown host (init_sockaddr()) = "
				<< hostname << endl;
		exit(EXIT_FAILURE);
	}

	name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}

void communication::receive_remote_msg(void * filedes) {

	statistics::total_num_received_msg++;

	packet_t *packet_rcv = (packet_t *) malloc(sizeof(packet_t));

	if (packet_rcv == NULL) {
		perror(
				"communication::receive_remote_msg(): error in malloc packet_rcv\n");
		exit(EXIT_FAILURE);
	};

	size_t nbytes;
	size_t mbytes;

	//size_t jbytes, kbytes;
	int connfd;

	connfd = *((int *) filedes);

	nbytes = read (connfd, (void*)packet_rcv, sizeof(packet_t));


	// Packet Type 1 means the packet is coming from remote host
	packet_rcv->data = (char *) malloc(packet_rcv->size);
	if (packet_rcv->data == NULL) {
		perror(
				"communication::receive_remote_msg(): error in malloc packet_rcv->data\n");
		exit(EXIT_FAILURE);
	};

	int pos = 0;
	int byte_to_read = packet_rcv->size;
	int count = 0;
	while (byte_to_read > 0) {
		mbytes = read(connfd, packet_rcv->data + pos, byte_to_read);

		if (mbytes < 0)
			cout
					<< "error in reading data from socket in communication thread"
					<< endl;

		pos = pos + mbytes;
		byte_to_read -= mbytes;
	}

	if (nbytes < sizeof(packet_t)) {
		perror(
				"communication::receive_remote_msg(): read socket error (receive_remote_msg())\n");
		exit(EXIT_FAILURE);
	} //end if
	else if (nbytes == 0)
		printf(
				"communication::receive_remote_msg(): received nbytes zero (receive_remote_msg()).");
	else {
		if (filedes)
			free(filedes);
	} //end else

	shutdown(connfd, 1);
	close(connfd);

	pcks->add_packet(packet_rcv);
	packets_received = packets_received++;
	return;

}

int communication::send_message(packet_t *p) {

	statistics::total_num_sent_msg++;

	if (p->message_type != 5)
		if (VERBOSE)
			cout << statistics::time_get_in_second() << ": " << "node "
					<< p->host_peer << " sending message_type " << p->message_type
					<< " to node " << p->dest_peer << " at port " << p->dest_port
					<< endl;

	char *buffer = NULL;
	int total_packet_size = 0;


	total_packet_size = (sizeof(packet_t) + (p->size));

	buffer = (char *) malloc(total_packet_size);

	if (buffer == NULL) {
		cout << "inside if buffer ==NULL" << endl;
		perror(
				"communication::send_message(): error in malloc() buffer (send_packet())\n");
		exit(EXIT_FAILURE);
	};

	if (buffer != NULL) {
		memcpy(buffer, p, sizeof(packet_t));
		if (p->size > 0) {
			memcpy((buffer + sizeof(packet_t)), p->data, p->size);
		}
	} //end if (buffer!= NULL)

	else {
		perror(
				"communication::send_message(): error in malloc() buffer (send_packet())\n");
		exit(EXIT_FAILURE);
	} //end else


	char* server_name = p->dest_machine;
	int server_port = p->dest_port;

	int sock;
	struct sockaddr_in server;

	//sock= socket(AF_INET, SOCK_DGRAM, 0);
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		error("socket");

	linger* set_linger = (linger*) malloc(sizeof(linger));

	init_sockaddr(&server, server_name, server_port);

	int j = 0;
	int attempt = 0;

	j = connect(sock, (struct sockaddr *) &server, sizeof(server));

	while (j < 0 && attempt < 10) {
		cout << "connection error: " << errno << endl;
		switch (errno) {
		case EBADF:
			perror("Descripteur de socket invalide.\n");
		case ENOTSOCK:
			perror("L'argument n'est pas une socket.\n");
		case EFAULT:
			perror(
					"Un paramètre pointe en dehors de l'espace d'adresssage accessible.\n");
		case EMSGSIZE:
			perror(
					"La socket nécessite une emission intégrale du message mais la taille de celui-ci ne le permet pas.\n");
		case EAGAIN:
			perror(
					"La socket est non-bloquante et l'opération demandée bloquerait.\n");
		case ENOBUFS:
			perror(
					"La file d'émission de l'interface réseau est pleine. Ceci indique généralement une panne de l'interface réseau, mais peut également être dû à un engorgement passager. Ceci ne doit pas se produire sous Linux, les paquets sont silencieusement éliminés. \n");
		case EINTR:
			perror("Un signal a été reçu. \n");
		case ENOMEM:
			perror("Pas assez de mémoire pour le noyau. \n");
		case EINVAL:
			perror("Un argument invalide a été transmis. \n");
		case EPIPE:
			perror(
					"L'écriture est impossible (correspondant absent). Dans ce cas le processus recevra également un signal SIGPIPE sauf s'il a activée l'option MSG_NOSIGNAL. \n");

		}
		sleep(5);
		j = connect(sock, (struct sockaddr *) &server, sizeof(server));
		attempt = attempt + 1;
	}

	if (j < 0) {
		cout
				<< "communication::send_message(): Connection Error: host_machine =  "
				<< p->host_machine << " host_peer = " << p->host_peer
				<< " dest_machine = " << p->dest_machine << " dest_peer = "
				<< p->dest_peer << endl;

		perror(
				"communication::send_message(): error connect from peer $p->host_peer (send_packet())\n");
		exit(EXIT_FAILURE);
	}

	int total_byte_wrote = 0;
	int num_byte_remain = total_packet_size;
	int count = 0;
	char *temp_buff = buffer;

	while (num_byte_remain > 0) {

		int nbytes = write(sock, temp_buff, num_byte_remain);

		if (nbytes < 0)
			cout << "error in writing data to the socket" << endl;

		num_byte_remain -= nbytes;
		temp_buff += nbytes;

	}

	shutdown(sock, 0);
	close(sock); // close connection socket

	if (p->message_type == 9){
		statistics::num_work_request_msg_sent++;
	}

	// msg type 3: work from a peer to its neighbor
	if(p->message_type == 3){
		statistics::num_work_reply_msg_sent++;
	}

	if (p != NULL) {
		free(p);
		if (buffer != NULL)
			free (buffer);
	}
	return 0;
}
