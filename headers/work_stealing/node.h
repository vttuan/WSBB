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
#include <time.h>
#include <deque>

#include "types.h"
#include "packets.h"
#include "communication.h"
#include "../pbab.h"
#include "statistics.h"

#include "../explorer.h"
#include "../gpu.h"

#include <pthread.h>

#ifndef NODE_H
#define NODE_H



//class works; class problem; class pbab;  
class Node
{
	public:
		Node();
		void serveWorkRequests(pbab*);

		void gpuServeWorkRequests1(pbab*);

		void rejectWorkRequest();
		bool gpuStealWorkFromGPUs(pbab*);
		void sendVoteTermination();
		bool gotAllVotesFromChildren();

		void broadcastTerminateSignalToGPUs();
		void broadcastTerminateSignalToCPUs();


		void recvJoiningData(packet_t*);
		void sendAddRequest();
		void recvNewChild(packet_t*);

		void recvWorkInWorkStealing(packet_t*, pbab*);
		void recvWorkInWorkStealingGPU(packet_t*, pbab*);
		void recvWorkInWorkStealingGPU1(packet_t*, pbab*);
		void recvWorkInWorkStealingGPU3(packet_t*, pbab*);

		void recvOptimalSolution(packet_t*, pbab*);
		void sendOptimalSolutionToGPUs(pbab* p);
		void sendOptimalSolutionToCPUs(pbab* p);

		void recvBroadcastIpMsg(packet_t*);
		void broadcastRestartTerrminateVote();
		void recvTerminationSignal();
		void sendJoinRequest();

		bool detectTermination(pbab*);
		/*
		 * GPU + CPU CLUSTER
		 */

		 // Workstealing
		void gpuServeWorkRequestsInWorkstealing(pbab* pbb);
		void cpuServeWorkRequestsInWorkstealing(pbab* pbb);

		void stealWork(pbab* pbb);
		void broadcastTerminateSignal();

		// CPU PRUNE+DECOMPOSE
		bool gotAllWRFromSlaves();
		void gpuRecvDecomposedProblems(packet_t* p, pbab* pbb);

		// CPU Decompose
		void serveWorkRequestsInCpuPruneDecompose (pbab* pbb);

		child_t* search_slave(peerid_t);
		void sendDecomposedSubProblemsToGPU(int sizeInElements, char* data);
		void recvDecomposedSubProblemsFromCPU(packet_t*, pbab* pbb);

		void sendAddRequestToGPUMaster();

		void recvCPUSlaves(packet_t*);

		bool gpuStealWorkFromCPUSlaves(pbab*);
		void gpuServeWorkRequests2(pbab*);

		bool cpuStealWorkFromGPUMaster(pbab*);
		bool cpuStealWorkFromGPUMaster(pbab*, int);

		bool cpuServeWorkRequestsOfMasterGPU(pbab*);

		void printOptimalSolution(pbab*);
		void printRandomPool();
		void printSlaveCPUs();

		child_t* searchChild(peerid_t);

		communication *com;
		string manager_machine;
		address_t *my_address;
		address_t *parent;

		child_t *last_child;
		child_t *first_child;
		int current_childs;

		deque<address_t> pendingWorkRequest;
		deque<child_t*> pendingWorkRequestFromMS;

		//Random pool of nodes
		child_t* random_pool_of_nodes;

		int current_number_of_peers_in_random_pool;
		int terminationSignal;
		int MTR;
		int jobFinished;
		int sentTerminateDetection;

		// GPU+CPU CLUSTER
		int computing_device;
		double computing_capability;
		child_t *last_slave;
		child_t *first_slave;
		child_t *next_serving_slave;


		int current_slaves;
		int MTS;				// Master to Slave
		int STM;				// Slave To Master
		int RTM;				// Random to Me

	private:
		bool terminateDetectionWorkStealing();
		// GPU + CPU CLUSTER
		bool sendMsg( int message_type, peerid_t dest_id, char* dest_machine, int dest_port, char* data, int data_size);
		bool sendMsg(int message_type, peerid_t dest_id, char* dest_machine, int dest_port, char* data, int data_size, int num_elements);
};
#endif

