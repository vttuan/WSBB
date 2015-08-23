#ifndef __types__
#define __types__

#include "../../int/BigInteger.hh"
#include "../../int/BigIntegerUtils.hh"
#include <stdint.h>
#include <time.h>

// Maximum buffer size to transfer between machines.
// file name of the file containing the list of machines. 
// static const char filename[]= "machines";

#define max_peers 500000
#define max_childs 1000 

#define manager_port 52000
#define heartbeat_duration 5

#define idle 0
#define exploring 1
#define paused 2
#define new_solution 3
#define bootstrap 4


#define VERBOSE 1
#define VERBOSE1 1

#define IS_SLAVE_PEER 	0
#define IS_RANDOM_PEER	1
#define IS_MASTER_PEER	2

// Use in Multi GPU-CPU WS
#define IS_GPU_PEER		3
#define IS_CPU_PEER		4


#define	SHARE_HALF		0.5
#define	SHARE_QUATER	0.25
#define	SHARE_HUIT		0.125

#define SHARE_GPU_GPU	0.5
#define SHARE_GPU_CPU	0.1
#define SHARE_CPU_GPU	0.9
#define SHARE_CPU_CPU	0.5

typedef int peerid_t;

typedef struct _peer {

	peerid_t id;
	char host_machine[100]; //name of the machine
	uint64_t listening_port;

	peerid_t parent_id;
	int number_of_childs;

	//int childs[max_childs];	
	int childs[max_childs];

	int height;

	_peer * previous_peer;
	_peer * next_peer;

} peer_t;

/******************************************************************/
typedef struct _address {

	peerid_t id; /**< Global id of the peer **/
	int listening_port; /**<listening port of this neighbor**/
	char host_machine[100]; /**< name of host machine where this neighbor lies **/
	int computeNodeCapability;
} address_t;

typedef int packetid_t;

/*
 message type

 # 0 parent/joining request from a new node to manager
 # 1 sending of peer id and parent address from manager to node
 # 2 add request from a new child to its parent
 # 3 work from a peer to its neighbor
 # 4 request for optimal solution value from a peer to its neighbor
 # 5 optimal solution value from a peer to its neighbor
 # 6 leave announcement from a peer to its neighbor
 # 7 check point request from a peer to its neighbor
 # 8 check point data from a peer to its neighbor
 # 9 work request from a peer to its neighbor
 #10 job_finished(termination) signal to childs
 #11 trigger signal from manager to the root node.
 #12 sub tree size request from a parent to its childs
 #13 tree size from a child to its parent

 #14 broadcast all nodes of the networks to the other nodes.
 #15 a rejection of to a work request (have no work to share)

 // For termination detection
 #16 vote for termination
 #18 repeat to vote for termination detection

// For GPU+CPU cluster
 #19 join request from CPU slave to GPU master

 //Manager will send this signal when the manager hass assigned the parent ids to the whole network.
 //Means when the network construction is completed.
 *
 */

#define ADD_REQUEST			2

#define BB_PROBLEM_PACKET		3
#define BB_DECOMPOSED_PROBLEM_PACKET	31
#define BB_PROBLEM_FOR_PRUNE_PACKET	32

#define OPTIMAL_SOLUTION_FOUND		5
#define WORK_REQUEST			9
#define TERMINATION_SIGNAL		10
#define REJECT_WORK_REQUEST		15
#define VOTE_TERMINATION		16
#define RESTART_VOTING_TERMINATION	18
#define ADD_REQUEST_TO_GPU_MASTER	19



typedef struct _packet {
	packetid_t id; 		/**< the packet id **/
	int message_type; 	/**< 0 means local packet, 1 means remote packet **/

	//destination address
	peerid_t dest_peer; 	/**<id of the destination peer**/
	char dest_machine[100]; /**< name of destination machine for the packet **/
	int dest_port; 			/**<port of the destination peer**/

	//source address
	peerid_t host_peer; 	/**<name of the originating peer**/
	char host_machine[100]; /**< name of host machine originating the packet **/
	int host_port; 		/**<listening port of the source peer**/

	//data values
	peerid_t data_id;	// manager sends info of parent to a peer when
				// it joins the network
	char data_machine[100];
	int data_port;

	int termination_signal; // 0: nothing, 1: positive vote, 2: negative vote

	int num_elements;	// num_elements in sending data
				// or id sequence of work request from GPU master
				// to CPU slaves
	int peer_type;		// GPU or CPU, it is used for sharing work

	int size; 		/**< size of the packet data, in bytes**/
	char *data; 		/**< packet data **/



} packet_t;

/******************************************************************/

//structure to contain the packets 
typedef struct _packet_container {

	packet_t * data_packet;
	_packet_container * next_container;
	_packet_container * previous_container;

} packet_container_t;

/******************************************************************/
typedef struct _child {
	peerid_t id; /**< Global id of the peer **/
	int listening_port; /**<listening port of this neighbor**/
	char host_machine[100]; /**< name of host machine where this neighbor lies **/

	int STM;
	int MTS;
	_child * next_sibling; /**<pointer to the next sibling. NUll means list is ended>**/
	_child * previous_sibling; /**<pointer to the previous sibling. NUll means list is ended>**/

	// for Work Stealing
	int termination_signal; // 0: nothing, 1: positive		2: negative
} child_t;

/******************************************************************/

/******************************************************************/
typedef struct _work {

} work_t;

#endif //__types__
