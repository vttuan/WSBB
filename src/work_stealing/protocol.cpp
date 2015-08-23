using namespace std;

#include <ctime>
#include <stdlib.h>
#include <iostream>

#include "../../int/BigInteger.hh"
#include "../../int/BigIntegerUtils.hh"
#include "../../headers/work_stealing/protocol.h"
#include "../../headers/explorer.h"

pthread_t listening_thread;

pthread_t gpu_exploration_thread;
pthread_t gpu_processing_thread;

pthread_t cpu_exploration_thread;
pthread_t cpu_gpu_exploration_thread;
pthread_t cpu_workstealing_processing_thread;
pthread_t cpu_prune_decompose_processing_thread;


long startTime;
long finishTime;
bool got_start_time_of_non_root = false;

int trigger = 0;

int root_iteration = 0;
int node_iteration = 0;

bool job_complete = false;
int duration = 5;

long startWaitingTimeForWork = 0;
Protocol::Protocol(int d, int peer_port, char* manager_machine, pbab* pbb) {
	duration = d;
	this->node = new Node();
	this->node->com->PORT = peer_port;
	this->node->my_address->listening_port = peer_port;

	this->node->manager_machine.clear();
	this->node->manager_machine.append(manager_machine);

	this->pbb = pbb;
	this->pbb->node = this->node;
}

void Protocol::error(const char* msg) {
	perror(msg);
	exit(0);
}

void* peerBootstrap(void *arg) {
	Protocol* protocol = (Protocol*) arg;

	int servsock_boot, *new_boot;
	struct sockaddr_in clientname_boot;
	size_t size;

	// Create the socket and set it to accept connections.
	servsock_boot = protocol->node->com->make_socket(protocol->node->com->PORT);
	if (listen(servsock_boot, 10000) < 0) {
		protocol->error(
				"peer_bootstrap.cpp: error in listen socket (peer_bootstrap())");
		exit(EXIT_FAILURE);
	}

	while (true) {

		if ((new_boot = (int *) malloc(sizeof(int))) == NULL) {
			protocol->error(
					"peer.cpp: malloc() error new_boot (peer_bootstrap())\n");
			exit(EXIT_FAILURE);
		}

		size = sizeof(struct sockaddr);
		*new_boot = accept(servsock_boot, (struct sockaddr *) &clientname_boot,
				(socklen_t *) &size);

		if (*new_boot < 0) {
			protocol->error("peer.cpp: error in accept (peer_bootstrap())\n");
			exit(EXIT_FAILURE);
		}

		protocol->node->com->receive_remote_msg(new_boot);

	}
	return NULL;
}

void cpuProcessingIncomingPackets(Protocol *protocol) {
	for (int j = 0; j < protocol->node->com->pcks->number_of_packets; j++) {
		// We extract all the packets waiting in the packets queue
		packet_t *p = protocol->node->com->pcks->get_packet();

		if (p != NULL) {

			// Receive joining data
			if (p->message_type == 1) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd joining data from node " << p->host_peer
							<< endl;

				protocol->node->recvJoiningData(p);

				if (VERBOSE)
					cout << statistics::time_get_in_second()
							<< ": after joining data worker_state = "
							<< protocol->pbb->workerState
							<< ", solution_state = "
							<< protocol->pbb->workerState << endl;

				//to bootstrap the root node
				if (protocol->node->my_address->id == 0) {
					if (VERBOSE)
						cout << statistics::time_get_in_second()
								<< "main::processing_bootstrap(): Initializing work for the root node"
								<< endl;
					protocol->pbb->tr->add_root();

				}

				protocol->pbb->wsltns->init();

				switch (protocol->pbb->cpu_operation){
					case CPU_WORKSTEALING:
						if (protocol->node->parent != NULL)
							protocol->node->sendAddRequest();
						break;

					default:
						if (protocol->node->my_address->id > 0 && protocol->node->computing_device == IS_GPU_PEER) {
							protocol->node->sendAddRequest();
							sleep(1);
						} else if (protocol->node->my_address->id > 0 && protocol->node->computing_device == IS_CPU_PEER){
							protocol->node->sendAddRequestToGPUMaster();
							sleep(1);
						};
						break;
				}
			}

			// Receive a new child
			else if (p->message_type == 2) {
				if (VERBOSE)
					cout << "node " << protocol->node->my_address->id
							<< " recvd a new child node " << p->host_peer
							<< endl;

				protocol->node->recvNewChild(p);
			}

			// Receive a new slave
			else if (p->message_type == 19) {
				if (VERBOSE)
					cout << "node " << protocol->node->my_address->id
							<< " recvd a slave child node " << p->host_peer
							<< endl;

				if (p->message_type == IS_GPU_PEER)
					protocol->node->recvCPUSlaves(p);
			}

			// Receive new work
			else if (p->message_type == BB_PROBLEM_PACKET) {
				// for statistics
				statistics::num_work_reply_msg_recv++;
				statistics::total_communication_time += (statistics::time_get_in_us() - startWaitingTimeForWork);
				startWaitingTimeForWork = 0;

				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd work BB_PROBLEM_PACKET from node " << p->host_peer << endl;

				if (protocol->pbb->cpu_operation == CPU_WORKSTEALING){
					protocol->node->MTR = 0;
					protocol->node->recvWorkInWorkStealingGPU3(p, protocol->pbb);
				}else{
					protocol->node->STM = 0;
					/*receive work*/
					//.............
					//.............
				}
			}

			else if (p->message_type == BB_PROBLEM_FOR_PRUNE_PACKET){
				if (VERBOSE)
						cout << statistics::time_get_in_second() << ": " << "node "
								<< protocol->node->my_address->id
								<< " recvd work BB_PROBLEM_FOR_PRUNE_PACKET from node " << p->host_peer << endl;

				cout << "BB_PROBLEM_FOR_PRUNE_PACKET: " << p->size << endl;
				cout << "BB_PROBLEM_FOR_PRUNE_PACKET: " << p->num_elements << endl;

				int startingIndex = 0;
				int fromHere = p->num_elements * sizeof(int);

				for (int i=0; i<p->num_elements; i++){
					int tmp_size = 0;
					memcpy(&tmp_size, p->data + i*sizeof(int), sizeof(int));

					cout << "BB_PROBLEM_FOR_PRUNE_PACKET: " << tmp_size << endl;

					cpu_raw_bb_problem* recv_problems = (cpu_raw_bb_problem*) malloc(sizeof(cpu_raw_bb_problem));
					recv_problems->size_int_byte = tmp_size;

					recv_problems->data = (raw_bb_problem*) malloc(recv_problems->size_int_byte);
					memcpy(recv_problems->data, p->data + fromHere + startingIndex, recv_problems->size_int_byte);

					startingIndex += recv_problems->size_int_byte;
					protocol->pbb->tr->babProblemsForPrune.push_back(recv_problems);

				}

				protocol->node->STM = 0;
			}

			// Receive optimal solution value
			else if (p->message_type == OPTIMAL_SOLUTION_FOUND) {
				if (VERBOSE1)
					cout << "node " << protocol->node->my_address->id
							<< " recvd optimal solution value from node "
							<< p->host_peer << endl;

				protocol->node->recvOptimalSolution(p, protocol->pbb);

				// check Optimal Solution here
				if (protocol->pbb->betterSolutionState == 1) {
					cout << statistics::time_get_in_second() << ": cout recv: " <<  protocol->pbb->tr->cout << endl;
					cout << statistics::time_get_in_second() << ": Optimal recv: " << *(protocol->pbb->wsltns) << endl;

					protocol->pbb->node->sendOptimalSolutionToGPUs(protocol->pbb);
					protocol->pbb->betterSolutionState = 0;

				}
			}

			// Receive work request
			else if (p->message_type == WORK_REQUEST) {
				statistics::num_work_request_msg_recv++;

				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd work request from node " << p->host_peer
							<< endl;

				switch (protocol->pbb->cpu_operation){
					case CPU_WORKSTEALING:
						_address new_request;
						new_request.id = p->host_peer;
						new_request.listening_port = p->host_port;
						new_request.computeNodeCapability = p->peer_type;

						memcpy(new_request.host_machine, p->host_machine, 100);

						protocol->node->RTM++;
						protocol->node->pendingWorkRequest.push_back(new_request);

						break;
					default:
						// work request from my GPU master
						if (p->host_peer == protocol->node->parent->id){
							protocol->node->MTS = 1;
						}
						else {
							_address new_request;
							new_request.id = p->host_peer;
							new_request.listening_port = p->host_port;
							memcpy(new_request.host_machine, p->host_machine, 100);

							protocol->node->RTM++;
							protocol->node->pendingWorkRequest.push_back(new_request);
						}
						break;
				}
			}

			//receive trigger signal from manager
			else if (p->message_type == 11) {

				// this message will be received only by the root node. and it will send tree size request in the network after getting the trigger signal.
				// trigger signal will be made true by the root node when it detects tree size calculation process termination.
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd a trigger signal from " << p->host_peer
							<< endl;

				startTime = statistics::time_get_in_us();
				trigger = 1;

				// Receive broadcast ip from manager
			} else if (p->message_type == 14) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd broadcast msg from manager "
							<< p->host_peer << endl;

				protocol->node->recvBroadcastIpMsg(p);

				if (VERBOSE)
					protocol->node->printRandomPool();

				// Receive a rejection from a random node to which I made a work request
			} else if (p->message_type == 15) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd rejection msg from " << p->host_peer
							<< endl;

				// reset this MTR flag for continuing send work request
				protocol->node->MTR = 0;

				// vote for termination signal
			} else if (p->message_type == 16) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd vote for termination detection from "
							<< p->host_peer << endl;

				child_t* child = protocol->node->searchChild(p->host_peer);
				if (child != NULL) {
					child->termination_signal = p->termination_signal;
					if (p->termination_signal == 2) {
						protocol->node->terminationSignal = p->termination_signal;
					}
				}

				// restart termination detection phase
			} else if (p->message_type == 18) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd restart for termination detection from "
							<< p->host_peer << endl;

				protocol->node->sentTerminateDetection = 0;
				protocol->node->terminationSignal = 0;

				protocol->node->broadcastRestartTerrminateVote();
			}
			// Receive termination signal
			else if (p->message_type == 10) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd termination signal from node "
							<< p->host_peer << endl;

				protocol->node->jobFinished = true;

				// child in work stealing tree
				protocol->node->broadcastTerminateSignal();

				protocol->node->printOptimalSolution(protocol->pbb);
				job_complete = true;

				/* statistics*/
				// GPU Computing
				if (protocol->pbb->xplr->type == 'g'){
					finishTime = statistics::time_get_in_us();
					statistics::total_execution_time = finishTime - startTime;
				}

				else{
					finishTime = statistics::time_get_in_us();
					statistics::total_execution_time = finishTime - startTime;
				}

				statistics::print();
				sleep(10);
				pthread_exit(0);
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

	}
}

void* cpu_processing_bootstrap(void *arg) {
	Protocol* protocol = (Protocol*) arg;

	while (!job_complete) {
		if (trigger == 1 && protocol->pbb->tr->size() > 0) {

			if (protocol->pbb->tr->size() > 0){
				protocol->pbb->setWorkerState(exploring);
				while (1) {

					cpuProcessingIncomingPackets(protocol);

					if (protocol->node->MTS == 1) {

						if (protocol->pbb->tr->babTreeForShare.size() > 0){
							long startCPUServingTime = statistics::time_get_in_us();
							//
							pthread_mutex_lock(&protocol->pbb->tr->bab_tree_mutex);
							//
							if (protocol->node->cpuServeWorkRequestsOfMasterGPU(protocol->pbb)){
								protocol->node->MTS = 0;
							}
							//
							pthread_mutex_unlock(&protocol->pbb->tr->bab_tree_mutex);

							long endCPUServingTime = statistics::time_get_in_us();
							statistics::total_CPU_serving_time += endCPUServingTime - startCPUServingTime;
						}
					}

					// GPU thread finish its work
					if (protocol->pbb->workerState == idle)
						break;

					usleep(100); // 0.01 second
				}

			}
		}else {
			if (trigger == 1 & protocol->pbb->workerState == idle) {

				if (protocol->node->STM == 0) {
					if (protocol->node->cpuStealWorkFromGPUMaster(protocol->pbb))
						protocol->node->STM = 1;
				}
			}

			cpuProcessingIncomingPackets(protocol);
		}

		usleep(100); // 0.01 second
	}
	return NULL;
}


void* cpu_workstealing_processing_bootstrap(void *arg) {
	Protocol* protocol = (Protocol*) arg;

		if (VERBOSE) {
			cout << "Worker Thread created" << endl;
		}

		while (!job_complete) {
			if (trigger == 1 && protocol->pbb->tr->size() > 0 &&  protocol->node->jobFinished == false) {

				if (protocol->pbb->tr->size() > 0){
					protocol->pbb->setWorkerState(exploring);
					while (1) {

						cpuProcessingIncomingPackets(protocol);

						if (protocol->node->pendingWorkRequest.size() > 0 && protocol->pbb->tr->size() > 0) {

								long startCPUServingTime = statistics::time_get_in_us();
								//
								protocol->node->cpuServeWorkRequestsInWorkstealing(protocol->pbb);

								//
								long endCPUServingTime = statistics::time_get_in_us();
								statistics::total_CPU_serving_time += endCPUServingTime - startCPUServingTime;
						}

						//
						protocol->pbb->xplr->_explore_cpu();

						// GPU thread finish its work
						if (protocol->pbb->tr->size() == 0){
							protocol->pbb->setWorkerState(idle);
							break;
						}
					}

				}
			} else {
				if (trigger == 1 && protocol->pbb->workerState == idle &&  protocol->node->jobFinished == false) {

					// for statistics
					if (startWaitingTimeForWork == 0)
						startWaitingTimeForWork = statistics::time_get_in_us();

					// neither positive vote nor negative vote
					if (protocol->node->terminationSignal == 0)
						protocol->node->terminationSignal = 1;

					if (protocol->node->pendingWorkRequest.size() > 0) {
						protocol->node->rejectWorkRequest();
					}

					// send random work request here (and root node checks termination)
					protocol->node->stealWork(protocol->pbb);

					// send terminate detection here
					if (protocol->node->my_address->id != 0
							&& protocol->node->gotAllVotesFromChildren()
							&& protocol->node->sentTerminateDetection == 0) {
						protocol->node->sendVoteTermination();
						protocol->node->sentTerminateDetection = 1;
					}

					// Root Node
					if (protocol->node->my_address->id == 0) {
						if (protocol->node->detectTermination(protocol->pbb)) {
							cout << "ROOT NODE: Job finished" << endl;
							protocol->node->jobFinished = true;
						}
					}

					// ROOT initializes a termination signal
					if (protocol->node->my_address->id == 0
							&& protocol->node->jobFinished) {

						cout << "main::processing_bootstrap(): inside if job_finished = true"<< endl;
						protocol->node->broadcastTerminateSignal();
						//
						cout << "main::processing_bootstrap(): node "
								<< protocol->node->my_address->id
								<< " exiting processing thread " << endl;

						protocol->node->printOptimalSolution(protocol->pbb);
						job_complete = true;

						// statistics
						finishTime = statistics::time_get_in_us();
						statistics::total_execution_time = finishTime - startTime;
						statistics::print();
						sleep(10);
					    pthread_exit(NULL);
					}
				}

				cpuProcessingIncomingPackets(protocol);
			}

			usleep(100); // 0.01 second
		}
		return NULL;
}

void* cpu_exploration_bootstrap(void *arg) {
	Protocol* protocol = (Protocol*) arg;

	while(!job_complete){
		if (trigger == 1 && protocol->pbb->workerState == exploring){
			cout << "CPU Exploring: " << protocol->pbb->tr->todo->begin << "  " << protocol->pbb->tr->todo->end << endl;

			switch (protocol->pbb->cpu_operation){
				case CPU_WORKSTEALING:
					protocol->pbb->xplr->_explore_cpu();
					break;

				case CPU_DECOMPOSE:
					protocol->pbb->xplr->cpu_explore_decomposing();
					break;

				case CPU_PRUNE_DECOMPOSE:
					protocol->pbb->xplr->cpu_pruneDecomposeProblem();
					break;

				case CPU_NORMAL_EXPLORE: break;
				case CPU_ADAPTIVE: break;
				default: cout << "CPU operation NOT_DEFINED" << endl; break;
			}

			protocol->pbb->setWorkerState(idle);
		}
		usleep(100);
	}
	return NULL;
}

void* cpu_prune_decompose_processing_bootstrap(void *arg) {
	Protocol* protocol = (Protocol*) arg;

		if (VERBOSE) {
			cout << "Worker Thread created" << endl;
		}

		while (!job_complete) {
			if (trigger == 1 && protocol->pbb->tr->babProblemsForPrune.size() > 0) {

				if (protocol->pbb->tr->babProblemsForPrune.size() > 0){
					protocol->pbb->setWorkerState(exploring);

					while(1){
						cpuProcessingIncomingPackets(protocol);
						// exploration thread finish its work
						if (protocol->pbb->workerState == idle)
							break;

						usleep(100); // 0.01 second
					}


				}
			} else {
				if (trigger == 1 & protocol->pbb->workerState == idle) {

					// steal work from GPU Master
					if (protocol->node->STM == 0){
						protocol->node->cpuStealWorkFromGPUMaster(protocol->pbb);
						protocol->node->STM = 1;
					}
				}

				cpuProcessingIncomingPackets(protocol);
			}

			usleep(100); // 0.01 second
		}
		return NULL;
}


void gpuProcessingIncomingPackets(Protocol *protocol) {
	for (int j = 0; j < protocol->node->com->pcks->number_of_packets; j++) {
		// We extract all the packets waiting in the packets queue
		packet_t *p = protocol->node->com->pcks->get_packet();

		if (p != NULL) {

			// Receive joining data
			if (p->message_type == 1) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd joining data from node " << p->host_peer
							<< endl;

				protocol->node->recvJoiningData(p);

				if (VERBOSE)
					cout << statistics::time_get_in_second()
							<< "after joining data worker_state= "
							<< protocol->pbb->workerState
							<< " solution_state = "
							<< protocol->pbb->workerState << endl;

				//to bootstrap the root node
				if (protocol->node->my_address->id == 0) {
					if (VERBOSE)
						cout << statistics::time_get_in_second()
								<< "main::processing_bootstrap(): Initializing work for the root node"
								<< endl;
					protocol->pbb->tr->add_root();

					if (protocol->pbb->workerState == idle)
						protocol->pbb->setWorkerState(exploring);
				}

				protocol->pbb->wsltns->init();

				if (protocol->node->my_address->id > 0 && protocol->node->computing_device == IS_GPU_PEER) {
					protocol->node->sendAddRequest();
					sleep(1);
				} else if (protocol->node->my_address->id > 0 && protocol->node->computing_device == IS_CPU_PEER){
					protocol->node->sendAddRequestToGPUMaster();
					sleep(1);
				}
			}

			// Receive a new child
			else if (p->message_type == 2) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": node " << protocol->node->my_address->id
							<< " recvd a new child node " << p->host_peer
							<< endl;

				protocol->node->recvNewChild(p);
			}

			// Receive a new slave
			else if (p->message_type == ADD_REQUEST_TO_GPU_MASTER) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << " node " << protocol->node->my_address->id
															<< " recvd a slave child node " << p->host_peer << endl;
				// GPU is a master of CPUs
				if (protocol->node->computing_device == IS_GPU_PEER)
					protocol->node->recvCPUSlaves(p);
			}

			// Receive new work
			else if (p->message_type == BB_PROBLEM_PACKET) {

				// for statistics
				statistics::num_work_reply_msg_recv++;
				statistics::total_communication_time += (statistics::time_get_in_us() - startWaitingTimeForWork);
				startWaitingTimeForWork = 0;

				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd work from node " << p->host_peer << endl;

				child_t* is_my_slave = protocol->node->search_slave(p->host_peer);
				// I received work from my slave
				if (is_my_slave != NULL){
					protocol->node->MTS = 0;
				}else{
					// I received work for the last random work request that I have sent
					protocol->node->MTR = 0;
					protocol->node->recvWorkInWorkStealingGPU3(p, protocol->pbb);

				}

				if (protocol->pbb->workerState == idle)
					protocol->pbb->setWorkerState(exploring);

			}

			// Receive new work
			else if (p->message_type == BB_DECOMPOSED_PROBLEM_PACKET) {

				// for statistics
				statistics::num_work_reply_msg_recv++;
				statistics::total_communication_time += (statistics::time_get_in_us() - startWaitingTimeForWork);
				startWaitingTimeForWork = 0;

				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd work from node " << p->host_peer << endl;

				child_t* slave = protocol->node->search_slave(p->host_peer);
				// if the work is from my slave
				if (slave != NULL){
					slave->MTS = 0;
					child_t *p = protocol->node->first_slave;
					child_t *p1 = p;

					while(p != NULL){
						if (p->MTS == 1)
							break;

						p1 = p1->next_sibling;
						p = p1;
					}

					// all slaves answer to wr of the GPU master
					if (p == NULL){
						protocol->node->MTS = 0;
					}
				}

				protocol->node->gpuRecvDecomposedProblems(p, protocol->pbb);

				if (protocol->pbb->workerState == idle)
					protocol->pbb->setWorkerState(exploring);

			}

			// Receive optimal solution value
			else if (p->message_type == OPTIMAL_SOLUTION_FOUND) {
				if (VERBOSE1)
					cout << "node " << protocol->node->my_address->id
							<< " recvd optimal solution value from node "
							<< p->host_peer << endl;

				protocol->node->recvOptimalSolution(p, protocol->pbb);
				// check Optimal Solution here
				if (protocol->pbb->betterSolutionState == 1) {
					cout << statistics::time_get_in_second() << ": cout recv: " <<  protocol->pbb->tr->cout << endl;
					cout << statistics::time_get_in_second() << ": Optimal recv: " << *(protocol->pbb->wsltns) << endl;

					protocol->pbb->node->sendOptimalSolutionToGPUs(protocol->pbb);
					protocol->pbb->node->sendOptimalSolutionToCPUs(protocol->pbb);

					protocol->pbb->betterSolutionState = 0;

				}
			}

			// Receive work request
			else if (p->message_type == WORK_REQUEST) {
				statistics::num_work_request_msg_recv++;

				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd work request from node " << p->host_peer
							<< endl;

				child_t* slave = protocol->node->search_slave(p->host_peer);
				// if the work request is from my slave
				if (slave != NULL){
					slave->STM = 1;
					protocol->node->pendingWorkRequestFromMS.push_back(slave);
				}else{
					_address new_request;
					new_request.id = p->host_peer;
					new_request.listening_port = p->host_port;
					new_request.computeNodeCapability = p->peer_type;
					memcpy(new_request.host_machine, p->host_machine, 100);

					protocol->node->pendingWorkRequest.push_back(new_request);
				}

				if (VERBOSE){
					cout << "pendingWorkRequestFromMS: " << protocol->node->pendingWorkRequestFromMS.size() << endl;
					cout << "pendingWorkRequest: " << protocol->node->pendingWorkRequest.size() << endl;
				}



			}

			//receive trigger signal from manager
			else if (p->message_type == 11) {

				// this message will be received only by the root node. and it will send tree size request in the network after getting the trigger signal.
				// trigger signal will be made true by the root node when it detects tree size calculation process termination.
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd a trigger signal from " << p->host_peer
							<< endl;

				trigger = 1;
				protocol->node->printSlaveCPUs();

				// Receive broadcast ip from manager
			} else if (p->message_type == 14) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd broadcast msg from manager "
							<< p->host_peer << endl;

				protocol->node->recvBroadcastIpMsg(p);

				if (VERBOSE)
					protocol->node->printRandomPool();

				// Receive a rejection from a random node to which I made a work request
			} else if (p->message_type == 15) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd rejection msg from " << p->host_peer
							<< endl;

				// reset this MTR flag for continuing send work request
				protocol->node->MTR = 0;

				// vote for termination signal
			} else if (p->message_type == 16) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd vote for termination detection from "
							<< p->host_peer << endl;

				child_t* child = protocol->node->searchChild(p->host_peer);
				if (child != NULL) {
					child->termination_signal = p->termination_signal;
					if (p->termination_signal == 2)
					{
						protocol->node->terminationSignal = p->termination_signal;
					}
				}

				// restart termination detection phase
			} else if (p->message_type == 18) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd restart for termination detection from "
							<< p->host_peer << endl;

				protocol->node->sentTerminateDetection = 0;
				protocol->node->terminationSignal = 0;

				protocol->node->broadcastRestartTerrminateVote();
			}
			// Receive termination signal
			else if (p->message_type == 10) {
				if (VERBOSE)
					cout << statistics::time_get_in_second() << ": " << "node "
							<< protocol->node->my_address->id
							<< " recvd termination signal from node "
							<< p->host_peer << endl;

				protocol->node->jobFinished = true;

				// child in work stealing tree
				protocol->node->broadcastTerminateSignal();

				// slave of the gpu master
				protocol->node->broadcastTerminateSignalToCPUs();

				protocol->node->printOptimalSolution(protocol->pbb);
				job_complete = true;

				/* statistics*/
				// GPU Computing
				if (protocol->pbb->xplr->type == 'g'){
					finishTime = statistics::time_get_in_us();
					statistics::total_execution_time = finishTime - startTime;
				}

				else{
					finishTime = statistics::time_get_in_us();
					statistics::total_execution_time = finishTime - startTime;
				}

				statistics::print();
				sleep(10);
				pthread_exit(0);
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

	}
}

void* gpu_processing_bootstrap(void *arg) {
	Protocol* protocol = (Protocol*) arg;
	static bool first = true;
	if(first)
	{
		protocol->pbb->gp->init();
		first = false;
	}

	while (!job_complete) {
		if (trigger == 1) {
			startTime = statistics::time_get_in_us();

			while (1 && protocol->node->jobFinished == false) {
				gpuProcessingIncomingPackets(protocol);
				protocol->pbb->xplr->_explore_cpu_gpu();

				// share work in WS
				if (protocol->node->pendingWorkRequest.size() > 0 && protocol->pbb->tr->size() > 0){
					long startCPUServingTime = statistics::time_get_in_us();
					//
					//
					protocol->node->gpuServeWorkRequestsInWorkstealing(protocol->pbb);
					//
					long endCPUServingTime = statistics::time_get_in_us();
					statistics::total_CPU_serving_time += endCPUServingTime - startCPUServingTime;

				}

				if (protocol->pbb->tr->size() == 0 &&
						protocol->pbb->gp->readyPoolToGPU.size() == 0 &&
							protocol->pbb->gp->outputPoolOfGPU.size() == 0)
				{
					protocol->pbb->setWorkerState(idle);
				}

				if (protocol->pbb->workerState == idle){
					// for statistics
					if (startWaitingTimeForWork == 0)
						startWaitingTimeForWork = statistics::time_get_in_us();

					// neither positive vote nor negative vote
					if (protocol->node->terminationSignal == 0)
						protocol->node->terminationSignal = 1;

					if (protocol->node->pendingWorkRequest.size() > 0) {
						protocol->node->rejectWorkRequest();
					}

					// send random work request here (and root node checks termination)
					if (protocol->node->MTR == 0) {
						protocol->node->stealWork(protocol->pbb);
					}

					// send terminate detection here
					if (protocol->node->my_address->id != 0
							&& protocol->node->gotAllVotesFromChildren()
							&& protocol->node->sentTerminateDetection == 0
							&& protocol->node->gotAllWRFromSlaves()) {

						protocol->node->sendVoteTermination();
						protocol->node->sentTerminateDetection = 1;
					}

					// Root Node
					if (protocol->node->my_address->id == 0) {
						if (protocol->node->detectTermination(protocol->pbb)) {
							cout << "ROOT NODE: Job finished" << endl;
							protocol->node->jobFinished = true;
						}
					}
					// ROOT initializes a termination signal
					if (protocol->node->my_address->id == 0
							&& protocol->node->jobFinished) {

						protocol->node->broadcastTerminateSignalToGPUs();
						protocol->node->broadcastTerminateSignalToCPUs();
						protocol->node->printOptimalSolution(protocol->pbb);
						job_complete = true;

						// statistics
						finishTime = statistics::time_get_in_us();
						statistics::total_execution_time = finishTime - startTime;
						statistics::print();
						sleep(10);
					    pthread_exit(NULL);
					}
					usleep(100); // 0.01 second
				}
			}
		}

		gpuProcessingIncomingPackets(protocol);
		usleep(100); // 0.01 second
	}
	return NULL;
}

void* gpu_exploration_bootstrap(void *arg) {
	Protocol* protocol = (Protocol*) arg;

	while(!job_complete){

		if (trigger == 1 && protocol->pbb->workerState == exploring){
			cout << "GPU Exploring" << endl;
			protocol->pbb->xplr->_explore_gpu();
		}

		usleep(100);
	}
	return NULL;
}

pthread_t* Protocol::run() {

	srand((unsigned int) time(0));

	// creating listening thread
	if (pthread_create(&listening_thread, NULL, peerBootstrap, this) == 0) {
		cout << "peer created: listening on port = " << this->node->com->PORT
				<< endl;
	} else {
		cout << "error in Peer creation in main()" << endl;
	}

	sleep(2);
	this->node->sendJoinRequest();

	// creating exploration thread of GPU
	if (this->pbb->xplr->type == 'g'){

		this->node->computing_device = IS_GPU_PEER;
		this->node->computing_capability = 0;
		/*
		 *  GPU Processing
		 */
		if (pthread_create(&gpu_processing_thread, NULL, gpu_processing_bootstrap, this) == 0) {
			cout << "GPU+CPU PROCESSING" << endl;
			sleep(2);
		} else {
			cout << "error in creation of  GPU processing thread in main()" << endl;
		}
		return &gpu_processing_thread;
	}

	// creating exploration thread of CPU
	else{
		this->node->computing_device = IS_CPU_PEER;
		this->node->computing_capability = 0;

		switch (this->pbb->cpu_operation){
			case CPU_WORKSTEALING:
				if (pthread_create(&cpu_workstealing_processing_thread, NULL,
						cpu_workstealing_processing_bootstrap, this) == 0) {
					cout << "WORKSTEALING CPU PROCESSING" << endl;
					sleep(2);
				} else {
					cout << "error in creation of  WORKSTEALING CPU PROCESSING thread in main()" << endl;
				}
				break;
			//
			case CPU_DECOMPOSE: break;
			//
			case CPU_PRUNE_DECOMPOSE:
				if (pthread_create(&cpu_prune_decompose_processing_thread, NULL, cpu_prune_decompose_processing_bootstrap, this) == 0) {
					cout << "CPU PRUNE DECOMPOSE PROCESSING" << endl;
					sleep(2);
				} else {
					cout << "error in creation of  CPU PRUNE DECOMPOSE PROCESSING thread in main()" << endl;
				}
				break;
			case CPU_NORMAL_EXPLORE: break;
			case CPU_ADAPTIVE: break;
			default: cout << "CPU operation NOT_DEFINED" << endl; break;
		}
		return &cpu_workstealing_processing_thread;
	}
}
