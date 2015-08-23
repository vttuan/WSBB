using namespace std;

#include "../../headers/work_stealing/packets.h"

pthread_mutex_t packet_container_mutex = PTHREAD_MUTEX_INITIALIZER;

packets::packets() {

	this->number_of_packets = 0;
	this->first_container = NULL;
	this->last_container = NULL;

}

int packets::add_packet(packet_t *p) {

	pthread_mutex_lock(&packet_container_mutex); //acquire the packet_container lock

	//We will always add the packet at the tail
	//create a container
	packet_container_t *pc = (packet_container_t *) malloc(sizeof(packet_container_t));
	pc->data_packet = p; //attach the container with the packet

	//increment the number of packets
	number_of_packets = number_of_packets + 1;

	//if it is the only packet at this moment
	if (number_of_packets == 1) {
		first_container = pc;
		last_container = pc;
		pc->next_container = NULL;
		pc->previous_container = NULL;
	}

	//if there exists already some packet, we add the packet at the end
	if (number_of_packets > 1) {
		pc->previous_container = last_container;
		pc->next_container = NULL;
		last_container->next_container = pc;
		last_container = pc;
	}

	pthread_mutex_unlock(&packet_container_mutex); //release the packet_container lock
	return 0;
}

packet_t * packets::get_packet() {

	pthread_mutex_lock(&packet_container_mutex); //acquire the packet_container lock
	packet_t *p = NULL;
	//We will always extract the packet from the head as implement FIFO

	if (number_of_packets > 0) { //to verify if there exists some packet

		//we assign the packet in the first container to P.
		p = first_container->data_packet; //update the pointer p as it will be returned to the calling function
		number_of_packets = number_of_packets - 1; //we decrement the number of packets

		if (number_of_packets == 0) { //if this was the last packet
			packet_container_t * tmp = first_container; //assign the first container to tmp
			first_container = NULL;
			last_container = NULL;
			free(tmp);
		} //endif(number_of_packets == 0)

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

int packets::get_number_of_packets() {

	return number_of_packets;
}
