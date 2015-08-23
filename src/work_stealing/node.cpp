using namespace std;

#include <time.h>
#include "../../headers/work_stealing/node.h"

Node::Node() {

	//creates a communication instance
	this->com = new communication(this);

	this->my_address = (address_t*) malloc(sizeof(address_t)); //structure to carry my own address
	this->my_address->id = -1;
	this->com->initialize_my_address(my_address); //initializes my_address structure

	this->parent = NULL;

	this->first_child = NULL;
	this->last_child = NULL;

	this->current_childs = 0;

	//Random Work Stealing
	this->current_number_of_peers_in_random_pool = 0;
	this->random_pool_of_nodes = NULL;
	this->MTR = 0;
	this->jobFinished = false;
	this->sentTerminateDetection = 0;
	this->terminationSignal = 0;

	// GPU CPU cluster
	this->computing_device = -1;		// Non defined computing device
	this->current_slaves = 0;
	this->STM = 0;
	this->MTS = 0;
	this->RTM = 0;


	this->next_serving_slave = NULL;
	this->first_slave = NULL;
	this->last_slave = NULL;
}

bool Node::sendMsg(int message_type, peerid_t dest_id, char* dest_machine, int dest_port, char* data, int data_size){
	//we are here if we have the work to send.
	packet_t *packet = (packet_t *) malloc(sizeof(packet_t));

	packet->host_peer = my_address->id;
	packet->host_port = my_address->listening_port;
	memcpy(packet->host_machine, my_address->host_machine, 100);

	packet->message_type = message_type; //means it is a work request from a peer to its neighbor

	packet->termination_signal = this->terminationSignal;

	packet->size = data_size;
	packet->data = data;
	packet->num_elements = 0;
	packet->peer_type = this->computing_capability;

	packet->dest_peer = dest_id;
	packet->dest_port = dest_port;
	memcpy(packet->dest_machine, dest_machine, 100);

	com->send_message(packet);

	return true;
}

bool Node::sendMsg(int message_type, peerid_t dest_id, char* dest_machine, int dest_port, char* data, int data_size, int num_elements){
	//we are here if we have the work to send.
	packet_t *packet = (packet_t *) malloc(sizeof(packet_t));

	packet->host_peer = my_address->id;
	packet->host_port = my_address->listening_port;
	memcpy(packet->host_machine, my_address->host_machine, 100);

	packet->message_type = message_type; //means it is a work request from a peer to its neighbor

	packet->termination_signal = this->terminationSignal;

	packet->size = data_size;
	packet->data = data;
	packet->num_elements = num_elements;
	packet->peer_type = this->computing_device;

	packet->dest_peer = dest_id;
	packet->dest_port = dest_port;
	memcpy(packet->dest_machine, dest_machine, 100);

	com->send_message(packet);

	return true;
}
void Node::sendOptimalSolutionToGPUs(pbab* pbb) {

	stringstream oss;
	oss.clear();

	oss << *(pbb->wsltns);

	string result;
	result.clear();
	result.append(oss.str());

	char* data = (char *) result.c_str();
	int sizeInBytes = result.size();

	// send to parent
	if (parent != NULL) {
		this->sendMsg(OPTIMAL_SOLUTION_FOUND, parent->id,  parent->host_machine, parent->listening_port, data, sizeInBytes);
	}

	// send to all the childs
	child_t * tmp_child = first_child;
	child_t * tmp = NULL;
	while (tmp_child != NULL) {

		this->sendMsg(OPTIMAL_SOLUTION_FOUND, tmp_child->id,  tmp_child->host_machine, tmp_child->listening_port, data, sizeInBytes);

		//we move to next child
		tmp = tmp_child->next_sibling;
		tmp_child = tmp;
	}
	return;
}

void Node::sendOptimalSolutionToCPUs(pbab* pbb) {
	stringstream oss;
	oss.clear();

	oss << *(pbb->wsltns);

	string result;
	result.clear();
	result.append(oss.str());

	char* data = (char *) result.c_str();
	int sizeInBytes = result.size();

	child_t * tmp_slave = first_slave;
	child_t * tmp = NULL;
	while (tmp_slave != NULL) {
		cout << tmp_slave->id << endl;

		this->sendMsg(OPTIMAL_SOLUTION_FOUND, tmp_slave->id,  tmp_slave->host_machine, tmp_slave->listening_port, data, sizeInBytes);

		// we move to next child
		tmp = tmp_slave->next_sibling;
		tmp_slave = tmp;
	}
	return;
}

void Node::stealWork(pbab* pbb) {
	if (this->MTR == 0 && this->current_number_of_peers_in_random_pool > 0) {
		int random_index = rand() % this->current_number_of_peers_in_random_pool;

		this->sendMsg(WORK_REQUEST, this->random_pool_of_nodes[random_index].id,
						this->random_pool_of_nodes[random_index].host_machine,
						this->random_pool_of_nodes[random_index].listening_port, NULL, 0);
		this->MTR = 1;
	}
}
void Node::gpuServeWorkRequests1(pbab* pbb) {
	cout << "serveWorkRequestsGPU3" << endl;

	bool serve_at_least_one_request = false;

	while (this->pendingWorkRequest.size() > 0) {

		if (pbb->tr->babTreeForShare.empty()) {
			break;
		}

		char* data = NULL;
		int sizeInBytes = 0;

		// Only 1 pending work request
		if (this->pendingWorkRequest.size() == 1){
			int sizeInElements = pbb->tr->babTreeForShare.size();
			data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5) * sizeInElements);
			int startingIndex = 0;

			for (int i=1; i <= sizeInElements; i++ ){
				raw_bb_problem* p = (pbb->tr->babTreeForShare.empty()? NULL: pbb->tr->babTreeForShare.front());
				if (p){
					pbb->tr->babTreeForShare.pop_front();

					memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
					sizeInBytes += sizeof(int)*MAX_NBJOBS;

					memcpy(data + sizeInBytes, &p->limite1, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->limite2, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->couts_somme, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->depth, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->begin_end, sizeof(int));
					sizeInBytes += sizeof(int);

					delete p;
				}
			}
		}
		// more than ONE pending work request
		else{
			int sizeInElements = 0;
			if (pbb->tr->babTreeForShare.size() > 1)
				sizeInElements = pbb->tr->babTreeForShare.size() / 2;
			else
				sizeInElements = 1;

			data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5) * sizeInElements);
			sizeInBytes = 0;
			for (int i=1; i <= sizeInElements; i++ ){
				raw_bb_problem* p = (pbb->tr->babTreeForShare.empty()? NULL: pbb->tr->babTreeForShare.front());

				if (p){
					pbb->tr->babTreeForShare.pop_front();

					memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
					sizeInBytes += sizeof(int)*MAX_NBJOBS;

					memcpy(data + sizeInBytes, &p->limite1, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->limite2, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->couts_somme, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->depth, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->begin_end, sizeof(int));
					sizeInBytes += sizeof(int);

					delete p;
				}
			}
		}

		if (sizeInBytes == 0 && data == NULL)
			break;

		cout << "Send Bytes: " << sizeInBytes << endl;
		_address pending_node = this->pendingWorkRequest.front();

		this->sendMsg(BB_PROBLEM_PACKET, pending_node.id,  pending_node.host_machine, pending_node.listening_port, data, sizeInBytes);

		this->pendingWorkRequest.pop_front();
		serve_at_least_one_request = true;
	}

	// send negative vote for termination detection to my parent/root
	if (serve_at_least_one_request) {
		this->terminationSignal = 2; // 2: negative_vote
	}
}

void Node::gpuServeWorkRequests2(pbab* pbb){
	cout << "serveWorkRequestsGPU3" << endl;

	/*
	 * Serving GPU in work stealing
	 */
	bool serve_at_least_one_request = false;

	while (this->pendingWorkRequest.size() > 0) {

		if (pbb->tr->babTreeForShare.empty()) {
			break;
		}

		char* data = NULL;
		int sizeInBytes = 0;

		// Only 1 pending work request
		if (this->pendingWorkRequest.size() == 1 && this->STM == 0){
			int sizeInElements = pbb->tr->babTreeForShare.size();
			data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5) * sizeInElements);
			int startingIndex = 0;

			for (int i=1; i <= sizeInElements; i++ ){
				raw_bb_problem* p = (pbb->tr->babTreeForShare.empty()? NULL: pbb->tr->babTreeForShare.front());
				if (p){
					pbb->tr->babTreeForShare.pop_front();

					memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
					sizeInBytes += sizeof(int)*MAX_NBJOBS;

					memcpy(data + sizeInBytes, &p->limite1, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->limite2, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->couts_somme, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->depth, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->begin_end, sizeof(int));
					sizeInBytes += sizeof(int);

					delete p;
				}
			}
		}
		// more than ONE pending work request
		else{
			int sizeInElements = 0;
			if (pbb->tr->babTreeForShare.size() > 1)
				sizeInElements = pbb->tr->babTreeForShare.size() / 2;
			else
				sizeInElements = 1;

			data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5) * sizeInElements);
			sizeInBytes = 0;
			for (int i=1; i <= sizeInElements; i++ ){
				raw_bb_problem* p = (pbb->tr->babTreeForShare.empty()? NULL: pbb->tr->babTreeForShare.front());

				if (p){
					pbb->tr->babTreeForShare.pop_front();

					memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
					sizeInBytes += sizeof(int)*MAX_NBJOBS;

					memcpy(data + sizeInBytes, &p->limite1, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->limite2, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->couts_somme, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->depth, sizeof(int));
					sizeInBytes += sizeof(int);

					memcpy(data + sizeInBytes, &p->begin_end, sizeof(int));
					sizeInBytes += sizeof(int);

					delete p;
				}
			}
		}

		if (sizeInBytes == 0 && data == NULL)
			break;

		cout << "Send Bytes: " << sizeInBytes << endl;

		_address pending_node = this->pendingWorkRequest.front();

		this->sendMsg(BB_PROBLEM_PACKET, pending_node.id,  pending_node.host_machine, pending_node.listening_port, data, sizeInBytes);

		this->pendingWorkRequest.pop_front();
		serve_at_least_one_request = true;
	}

	// send negative vote for termination detection to my parent/root
	if (serve_at_least_one_request) {
		this->terminationSignal = 2; // 2: negative_vote
	}

	/*
	 * Serving my slaves
	 */

	while(this->STM != 0){

		if (pbb->tr->babTreeForShare.empty()) {
			break;
		}

		char* data = NULL;
		int sizeInBytes = 0;

		// if this slave sent me a work request
		if (this->next_serving_slave->STM == 1){

			if (this->STM == 1){
				int sizeInElements = pbb->tr->babTreeForShare.size();
				data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5) * sizeInElements);
				int startingIndex = 0;

				for (int i=1; i <= sizeInElements; i++ ){
					raw_bb_problem* p = (pbb->tr->babTreeForShare.empty()? NULL: pbb->tr->babTreeForShare.front());
					if (p){
						pbb->tr->babTreeForShare.pop_front();

						memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
						sizeInBytes += sizeof(int)*MAX_NBJOBS;

						memcpy(data + sizeInBytes, &p->limite1, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->limite2, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->couts_somme, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->depth, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->begin_end, sizeof(int));
						sizeInBytes += sizeof(int);

						delete p;
					}
				}
			}else{
				int sizeInElements = 0;
				if (pbb->tr->babTreeForShare.size() > 1)
					sizeInElements = pbb->tr->babTreeForShare.size() / 2;
				else
					sizeInElements = 1;

				data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5) * sizeInElements);
				sizeInBytes = 0;
				for (int i=1; i <= sizeInElements; i++ ){
					raw_bb_problem* p = (pbb->tr->babTreeForShare.empty()? NULL: pbb->tr->babTreeForShare.front());

					if (p){
						pbb->tr->babTreeForShare.pop_front();

						memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
						sizeInBytes += sizeof(int)*MAX_NBJOBS;

						memcpy(data + sizeInBytes, &p->limite1, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->limite2, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->couts_somme, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->depth, sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &p->begin_end, sizeof(int));
						sizeInBytes += sizeof(int);

						delete p;
					}
				}
			}

			if (sizeInBytes == 0 && data == NULL)
				break;

			this->sendMsg(BB_PROBLEM_PACKET, this->next_serving_slave->id,  this->next_serving_slave->host_machine, this->next_serving_slave->listening_port, data, sizeInBytes);

			this->next_serving_slave->STM = 0;
			this->STM--;
		}

		// traversing slaves in round robin maner
		this->next_serving_slave = this->next_serving_slave->next_sibling;
		if (this->next_serving_slave == NULL)
			this->next_serving_slave = this->first_slave;

	}

	cout << "after share: " << this->STM << endl;
}

void Node::gpuServeWorkRequestsInWorkstealing(pbab* pbb) {

	bool serve_at_least_one_request = false;

	while (this->pendingWorkRequest.size() > 0) {

		if (pbb->tr->empty() || pbb->tr->size() == 1) {
			break;
		}

		char* data = NULL;
		int sizeInBytes;

		int sizeInElements = 0;

		if (pbb->tr->size() > 1){

			_address pending_node = this->pendingWorkRequest.front();

			double fraction = 0.0;
			if (pending_node.computeNodeCapability == 0)
				fraction = 0.5;
			else{
				if (this->computing_capability == pending_node.computeNodeCapability)
					fraction = 0.5;
				else
					fraction = this->computing_capability / (this->computing_capability + pending_node.computeNodeCapability);
			}

			sizeInElements = pbb->tr->size() * fraction;

			if (VERBOSE){
				cout << "this->computing_capability: " << this->computing_capability << endl;
				cout << "pending_node.computeNodeCapability: " << pending_node.computeNodeCapability << endl;

				cout << "share fraction: " << fraction << endl;
				cout << "tree size: " << pbb->tr->size() << endl;
				cout << "share: " << sizeInElements << endl;
			}


			if (sizeInElements == 0)
				sizeInElements = 1;

			if (sizeInElements > 0){
				data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5+(MAX_NBJOBS+1)) * sizeInElements);
				sizeInBytes = 0;

				for (int i=1; i <= sizeInElements; i++ ){
					raw_bb_problem* p = (pbb->tr->empty()? NULL: pbb->tr->take());

					if (p){
						memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
						sizeInBytes += (sizeof(int) * MAX_NBJOBS);

						memcpy(data + sizeInBytes, &(p->limite1), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->limite2), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->couts_somme), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->depth), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->begin_end), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, p->ranks, sizeof(int) * (MAX_NBJOBS+1));
						sizeInBytes += (sizeof(int) * (MAX_NBJOBS+1));

						free (p);
					}
				}

				if (sizeInBytes == 0 || data == NULL)
					break;


				this->pendingWorkRequest.pop_front();

				this->RTM--;

				this->sendMsg(BB_PROBLEM_PACKET, pending_node.id,  pending_node.host_machine, pending_node.listening_port, data, sizeInBytes);
				free(data);
				serve_at_least_one_request = true;
			}

		}
	}

	// send negative vote for termination detection to my parent/root
	if (serve_at_least_one_request) {
		this->terminationSignal = 2; // 2: negative_vote
	}
}

void Node::cpuServeWorkRequestsInWorkstealing(pbab* pbb) {

	bool serve_at_least_one_request = false;

	while (this->pendingWorkRequest.size() > 0) {

		if (pbb->tr->empty() || pbb->tr->size() == 1) {
			break;
		}

		char* data = NULL;
		int sizeInBytes;

		int sizeInElements = 0;

		if (pbb->tr->size() > 1){

			_address pending_node = this->pendingWorkRequest.front();

			double fraction = 0.0;

			if (pending_node.computeNodeCapability == 0)
				fraction = 0.5;
			else{
				if (this->computing_capability == pending_node.computeNodeCapability)
					fraction = 0.5;
				else
					fraction = this->computing_capability / (this->computing_capability + pending_node.computeNodeCapability);
			}

			sizeInElements = pbb->tr->size() * fraction;

			if (VERBOSE){
				cout << "this->computing_capability: " << this->computing_capability << endl;
				cout << "pending_node.computeNodeCapability: " << pending_node.computeNodeCapability << endl;

				cout << "share fraction: " << fraction << endl;
				cout << "tree size: " << pbb->tr->size() << endl;
				cout << "share: " << sizeInElements << endl;
			}
			if (sizeInElements == 0)
				sizeInElements = 1;

			if (sizeInElements > 0){
				data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5+(MAX_NBJOBS+1)) * sizeInElements);
				sizeInBytes = 0;

				for (int i=1; i <= sizeInElements; i++ ){
					raw_bb_problem* p = (pbb->tr->empty()? NULL: pbb->tr->take());

					if (p){
						memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
						sizeInBytes += (sizeof(int) * MAX_NBJOBS);

						memcpy(data + sizeInBytes, &(p->limite1), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->limite2), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->couts_somme), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->depth), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, &(p->begin_end), sizeof(int));
						sizeInBytes += sizeof(int);

						memcpy(data + sizeInBytes, p->ranks, sizeof(int) * (MAX_NBJOBS+1));
						sizeInBytes += (sizeof(int) * (MAX_NBJOBS+1));

						free (p);
					}
				}

				if (sizeInBytes == 0 || data == NULL)
					break;

				this->pendingWorkRequest.pop_front();

				this->RTM--;

				this->sendMsg(BB_PROBLEM_PACKET, pending_node.id,  pending_node.host_machine, pending_node.listening_port, data, sizeInBytes);
				free(data);
				serve_at_least_one_request = true;
			}

		}
	}

	// send negative vote for termination detection to my parent/root
	if (serve_at_least_one_request) {
		this->terminationSignal = 2; // 2: negative_vote
	}
}

//void Node::serveWorkRequestsInCpuPruneDecompose(pbab* pbb){
//
//	while (pbb->tr->outputGPUForShare.size() > 0){
//		problemPool* p = pbb->tr->outputGPUForShare.front();
//		int sizeInBytes = 0;
//		sizeInBytes =  sizeof(raw_bb_problem) * p->children_size;
//
//		if (p->children_size > 0){
//			if (this->pendingWorkRequestFromMS.size() > 0){
//				_address pending_node = this->pendingWorkRequestFromMS.front();
//				// remove pending_node
//				this->pendingWorkRequestFromMS.pop_front();
//				this->STM--;
//
//				 // for termination in MS of GPU-CPU
//				 this->served_slave = true;
//
//				this->sendMsg(BB_PROBLEM_FOR_PRUNE_PACKET, pending_node.id,  pending_node.host_machine,
//								pending_node.listening_port, (char*)p->children_d, sizeInBytes);
//
//
//
//				// remove BB_PROBLEM_FOR_PRUNE_PACKET
//				 free(p->children_d);
//				 free(p->children_bounds);
//				 delete p;
//				 pbb->tr->outputGPUForShare.pop_front();
//
//
//
//			}else{
//				// serve automatically my slave
//				if (next_serving_slave != NULL){
//					 // for termination in MS of GPU-CPU
//					 this->served_slave = true;
//
//					this->sendMsg(BB_PROBLEM_FOR_PRUNE_PACKET, next_serving_slave->id,  next_serving_slave->host_machine,
//									next_serving_slave->listening_port, (char*)p->children_d, sizeInBytes);
//
//					//
//					if (next_serving_slave == last_slave)
//						next_serving_slave = first_slave;
//					else
//						next_serving_slave = next_serving_slave->next_sibling;
//
//					// remove BB_PROBLEM_FOR_PRUNE_PACKET
//					 free(p->children_d);
//					 free(p->children_bounds);
//					 delete p;
//					 pbb->tr->outputGPUForShare.pop_front();
//
//
//				}
//			}
//		}
//	}
//}

void Node::serveWorkRequestsInCpuPruneDecompose(pbab* pbb){

	while (pbb->tr->outputGPUForShare.size() > 0 && this->pendingWorkRequestFromMS.size() > 0){
		int share_items = 0;
		if (pbb->tr->outputGPUForShare.size() < this->pendingWorkRequestFromMS.size())
			share_items = 1;
		else
			share_items = (int) (pbb->tr->outputGPUForShare.size() / this->pendingWorkRequestFromMS.size());

		char* data =  (char*) malloc(sizeof(int) * share_items + pbb->gp->pool_size * sizeof(raw_bb_problem) * share_items);
		int sizeInBytes1 = 0;
		int sizeInBytes2 = 0;

		int startIndex = share_items * sizeof(int);

		for (int j=0; j<share_items; j++){
			problemPool* p = pbb->tr->outputGPUForShare.front();
			int tmp =  sizeof(raw_bb_problem) * p->children_size;

			memcpy (data + j * sizeof(int), &tmp, sizeof(int));
			sizeInBytes1 += sizeof(int);

			memcpy(data + startIndex + sizeInBytes2, p->children_d, tmp);
			sizeInBytes2 += tmp;

			// remove BB_PROBLEM_FOR_PRUNE_PACKET
			 free(p->children_d);
			 free(p->children_bounds);
			 delete p;

			 pbb->tr->outputGPUForShare.pop_front();
		}

		// serve pending work request
		if (this->pendingWorkRequestFromMS.size() > 0){
			child_t* next_serving_slave = this->pendingWorkRequestFromMS.front();

			this->sendMsg(BB_PROBLEM_FOR_PRUNE_PACKET, next_serving_slave->id,  next_serving_slave->host_machine,
						next_serving_slave->listening_port, data, sizeInBytes1 + sizeInBytes2, share_items);

			next_serving_slave->STM = 0;
			this->pendingWorkRequestFromMS.pop_front();

			cout << "serveWorkRequestsInCpuPruneDecompose, share items: " << share_items << endl;
		}
		free(data);
	}
}

bool Node::cpuServeWorkRequestsOfMasterGPU(pbab* pbb){
	int sizeInElements = pbb->tr->babTreeForShare.size();
	char* data = (char*)malloc(sizeof(int)*(MAX_NBJOBS+5) * sizeInElements);
	int sizeInBytes = 0;

	for (int i=1; i <= sizeInElements; i++ ){
		raw_bb_problem* p = (pbb->tr->babTreeForShare.empty()? NULL: pbb->tr->babTreeForShare.front());

		if (p){
			pbb->tr->babTreeForShare.pop_front();

			memcpy(data + sizeInBytes, p->permutation, sizeof(int)*MAX_NBJOBS);
			sizeInBytes += sizeof(int)*MAX_NBJOBS;

			memcpy(data + sizeInBytes, &p->limite1, sizeof(int));
			sizeInBytes += sizeof(int);

			memcpy(data + sizeInBytes, &p->limite2, sizeof(int));
			sizeInBytes += sizeof(int);

			memcpy(data + sizeInBytes, &p->couts_somme, sizeof(int));
			sizeInBytes += sizeof(int);

			memcpy(data + sizeInBytes, &p->depth, sizeof(int));
			sizeInBytes += sizeof(int);

			memcpy(data + sizeInBytes, &p->begin_end, sizeof(int));
			sizeInBytes += sizeof(int);

			delete p;
		}
	}

	cout << "size in Elements: " << sizeInElements << endl;
	cout << "size in Byte: " << sizeInBytes << endl;

	if (sizeInBytes == 0 && data == NULL)
		return false;

	this->sendMsg(BB_PROBLEM_PACKET, this->parent->id,  this->parent->host_machine, this->parent->listening_port, data, sizeInBytes);
	return true;
}

void Node::sendDecomposedSubProblemsToGPU(int sizeInElements, char* data){
	int sizeInBytes = sizeInElements * sizeof(raw_bb_problem);
	this->sendMsg(BB_DECOMPOSED_PROBLEM_PACKET, this->parent->id,  this->parent->host_machine, this->parent->listening_port, data, sizeInBytes);
}

void Node::rejectWorkRequest() {
	while (this->pendingWorkRequest.size() > 0) {
		_address pending_node = this->pendingWorkRequest.front();
		this->sendMsg(REJECT_WORK_REQUEST, pending_node.id,  pending_node.host_machine, pending_node.listening_port, NULL, 0);
		this->pendingWorkRequest.pop_front();
	}
}
bool Node::terminateDetectionWorkStealing() {
	child_t * tmp_child = first_child;
	child_t * tmp;

	// Root got negative termination or not
	bool negative_termination = false;
	while (tmp_child != NULL) {
		if (tmp_child->termination_signal == 0)
			return false;
		else if (tmp_child->termination_signal == 2)
			negative_termination = true;

		//else we move to next child
		tmp = tmp_child->next_sibling;
		tmp_child = tmp;
	}

	// If I am here, all my children sent termination signal to me already
	if (negative_termination == false && this->terminationSignal == 1)
		return true;
	else {

		//reset the flag of negative vote in the root
		this->terminationSignal = 0;
		child_t * tmp_child1 = first_child;
		child_t * tmp1;

		while (tmp_child1 != NULL) {

			tmp_child1->termination_signal = 0;

			this->sendMsg(RESTART_VOTING_TERMINATION, tmp_child1->id,  tmp_child1->host_machine, tmp_child1->listening_port, NULL, 0);

			//else we move to next child
			tmp1 = tmp_child1->next_sibling;
			tmp_child1 = tmp1;
		}

		// repeat to vote for termination
		return false;
	}
}

bool Node::gotAllWRFromSlaves(){
	child_t *p = this->first_slave;
	child_t *p1 = p;

	while(p != NULL){
		if (p->STM == 0)
			return false;

		p1 = p1->next_sibling;
		p = p1;
	}
	return true;
}
bool Node::detectTermination(pbab* pbb){
	if (my_address->id == 0) {
		if (terminateDetectionWorkStealing() && gotAllWRFromSlaves()) {
			if (pbb->workerState != exploring) {
				cout << "ROOT NODE: Job finished" << endl;
				jobFinished = true;
				cout << "inside send_work_request job_finished after being true = "
						<< jobFinished << endl;
			}
			return true;
		}
	}

	return false;
}
bool Node::gpuStealWorkFromGPUs(pbab* pbb) {
	// Root Node
	if (this->current_number_of_peers_in_random_pool > 0){
		int random_index = rand() % this->current_number_of_peers_in_random_pool;

		this->sendMsg(WORK_REQUEST, this->random_pool_of_nodes[random_index].id,  this->random_pool_of_nodes[random_index].host_machine,
				this->random_pool_of_nodes[random_index].listening_port, NULL, 0);

		return true;
	}
	return false;
}


bool Node::cpuStealWorkFromGPUMaster(pbab* pbb) {
	if (this->STM == 0) {
		this->sendMsg(WORK_REQUEST, this->parent->id,  this->parent->host_machine, this->parent->listening_port, NULL, 0);
		return true;
	}
	return false;
}

bool Node::cpuStealWorkFromGPUMaster(pbab* pbb, int sequence_id) {
	this->sendMsg(WORK_REQUEST, this->parent->id,  this->parent->host_machine, this->parent->listening_port, NULL, 0, sequence_id);
	return true;

}

bool Node::gpuStealWorkFromCPUSlaves(pbab* pbb){
	child_t * tmp_slave = first_slave;
	child_t * tmp = NULL;
	bool sent_to_a_slave = false;

	while(tmp_slave != NULL){
		this->sendMsg(WORK_REQUEST, tmp_slave->id,  tmp_slave->host_machine, tmp_slave->listening_port, NULL, 0);

		// Move to next slave
		tmp = tmp_slave->next_sibling;
		tmp_slave = tmp;
	}

	return true;
}

void Node::sendVoteTermination(){
	this->sendMsg(VOTE_TERMINATION, this->parent->id,  this->parent->host_machine, this->parent->listening_port, NULL, 0);
}

bool Node::gotAllVotesFromChildren(){
	child_t * tmp_child = first_child;
	child_t * tmp=NULL;
	while(tmp_child!=NULL){

		if (tmp_child->termination_signal == 0)
			return false;

		//we move to next child
		tmp = tmp_child->next_sibling;
		tmp_child=tmp;
	}

	return true;
}

void Node::broadcastTerminateSignalToGPUs(){
	// send termination signal to all the childs

	child_t * tmp_child = first_child;
	child_t * tmp = NULL;
	while(tmp_child != NULL){
		this->sendMsg(TERMINATION_SIGNAL, tmp_child->id,  tmp_child->host_machine, tmp_child->listening_port, NULL, 0);

		//we move to next child
		tmp = tmp_child->next_sibling;
		tmp_child = tmp;
	}
	cout<<"node " << my_address->id << " sendig termination signal to its childs." << endl;
}

void Node::broadcastTerminateSignalToCPUs(){
	// send termination signal to all the childs

	child_t * tmp_child = first_slave;
	child_t * tmp = NULL;
	while(tmp_child != NULL){
		this->sendMsg(TERMINATION_SIGNAL, tmp_child->id,  tmp_child->host_machine, tmp_child->listening_port, NULL, 0);

		//we move to next child
		tmp=tmp_child->next_sibling;
		tmp_child=tmp;
	}
	cout<<"node " << my_address->id << " sendig termination signal to its childs." << endl;
}

void Node::broadcastTerminateSignal(){
	//send termination signal to all the childs
	child_t * tmp_child= first_child;
	child_t * tmp=NULL;
	while(tmp_child!=NULL){

		this->sendMsg(TERMINATION_SIGNAL, tmp_child->id,  tmp_child->host_machine, tmp_child->listening_port, NULL, 0);

		//we move to next child
		tmp=tmp_child->next_sibling;
		tmp_child=tmp;
	}
	cout<<"node " << my_address->id << " sendig termination signal to its childs." << endl;
}
void Node::printOptimalSolution(pbab* pbb){
	stringstream oss;
	oss.clear();
	oss << *(pbb->wsltns);
	cout << "node::optimal_solution_value = " << oss.str() << endl;
	return;
}

void Node::recvJoiningData(packet_t* p){
	my_address->id = p->dest_peer; //gets peer_id assigned by the manager

	// to make sure I am not root node
	if(my_address->id > 0){
		this->parent = (address_t *)malloc(sizeof(address_t));
		parent->id = p->data_id;
		memcpy(parent->host_machine, p->data_machine, 100);
		parent->listening_port=p->data_port;

		cout<<"joining data received from manager"<<endl;
		cout<<"my_id = "<< my_address->id<<" parent_id = "<<parent->id<<" parent_machine="<<parent->host_machine<<" parent_port="<<parent->listening_port<<endl;
	}
	// if I am root node
	else{
		parent=NULL;
	}
	cout << "My peer ID : " << my_address->id << endl;
}

void Node::sendAddRequest(){
	if(parent->host_machine != NULL){
		this->sendMsg(ADD_REQUEST, parent->id,  parent->host_machine, parent->listening_port, NULL, 0);
	}
}

void Node::sendAddRequestToGPUMaster(){

	if(parent->host_machine != NULL){
		this->sendMsg(ADD_REQUEST_TO_GPU_MASTER, parent->id,  parent->host_machine, parent->listening_port, NULL, 0);

	}
}

void Node::recvNewChild(packet_t* p){
	if(my_address->id != p->host_peer){//to not to add itself as child

		current_childs = current_childs + 1;//increments the number of childs
		child_t * new_child = (child_t *)malloc(sizeof(child_t));

		new_child->id = p->host_peer;
		new_child->listening_port = p->host_port;
		new_child->STM = 0;

		// for work stealing
		new_child->termination_signal = 0;

		memcpy(new_child->host_machine, p->host_machine, 100);

		// if it is a first child
		if(current_childs == 1){
			first_child = new_child;
			last_child = new_child;
			new_child->previous_sibling=NULL;
			new_child->next_sibling=NULL;
		}

		// if it is not a first child
		if(current_childs > 1){
			new_child->previous_sibling = last_child;	//connects it to the previous sibling which is last child
			new_child->next_sibling = NULL;
			last_child->next_sibling = new_child;		//connects the previous sibling to this new_child
			last_child = new_child;						//updates the last last_child pointer to point to the current child
		}

		cout<<"child added id = "<<last_child->id<<" port = "<<last_child->listening_port<<" machine = "<<last_child->host_machine<<endl;
		cout<<"total number of childs = "<<current_childs<<endl;

	}

	return;
}

void Node::recvCPUSlaves(packet_t* p){
	if(my_address->id != p->host_peer){

		current_slaves = current_slaves + 1;
		child_t * new_child = (child_t *)malloc(sizeof(child_t));

		new_child->id = p->host_peer;
		new_child->listening_port = p->host_port;
		new_child->STM = 0;

		memcpy(new_child->host_machine, p->host_machine, 100);

		// if it is a first slave
		if(current_slaves == 1){
			first_slave = new_child;
			last_slave = new_child;
			new_child->previous_sibling=NULL;
			new_child->next_sibling=NULL;

			// for update next slave to serve
			this->next_serving_slave = first_slave;
		}else if (current_slaves > 1){
			new_child->previous_sibling = last_slave;	//connects it to the previous sibling which is last child
			new_child->next_sibling = NULL;
			last_slave->next_sibling = new_child;		//connects the previous sibling to this new_child
			last_slave = new_child;						//updates the last last_child pointer to point to the current child
		}

		cout << "slave added id = " << last_slave->id <<" port = " <<
				last_slave->listening_port << " machine = " << last_slave->host_machine<<endl;

		cout<<"total number of slave = " << current_slaves << endl;
	}
	return;
}

child_t* Node::search_slave(peerid_t slave_id){
	child_t *tmp_slave = this->first_slave;
	child_t *tmp;

	while(tmp_slave!=NULL){
		if (tmp_slave->id == slave_id)
			return tmp_slave;

		tmp = tmp_slave->next_sibling;
		tmp_slave = tmp;
	}
	return NULL;
}

void Node::recvWorkInWorkStealing(packet_t* p, pbab* pbb){
	string str, begin, end;
	str.clear();
	str.append(p->data,p->size);

//	if (VERBOSE)
//		cout << "node::recv_work(): received work = " << str << endl;

	istringstream stream(str);
	begin.clear();
	end.clear();

	stream >> begin;
	stream >> end;

	BigInteger b = easyStringToBI(begin);
	BigInteger e = easyStringToBI(end);

	work *tmp = new work(b,e, pbb);

	//I want to check the interval I am receiving is not empty.
	if(tmp->empty())
		return;

	pbb->wrks->id_update(tmp);
	pbb->wrks->sizes_update(tmp);
	pbb->wrks->times_update(tmp);
}

void Node::recvWorkInWorkStealingGPU(packet_t* p, pbab* pbb){
//	string str, begin, end;
//	str.clear();
//	str.append(p->data,p->size);
//
//	istringstream stream(str);
//
////	if (VERBOSE)
////		cout << "node::recv_work(): received work = " << str << endl;
//
//	cout << "numberOfInterval: " << p->numberOfInterval << endl;
//	for(int i=0; i<p->numberOfInterval; i++){
//		begin.clear(); end.clear();
//
//		stream >> begin;
//		stream >> end;
//
//		BigInteger b = easyStringToBI(begin);
//		BigInteger e = easyStringToBI(end);
//
//		work *tmp = new work(b,e, pbb);
//
//		//I want to check the interval I am receiving is not empty.
//		if(tmp->empty())
//			delete tmp;
//		else{
//			pbb->wrks->id_update(tmp);
//			pbb->wrks->sizes_update(tmp);
//			pbb->wrks->times_update(tmp);
//		}
//	}
}

void Node::recvWorkInWorkStealingGPU1(packet_t* p, pbab* pbb){
//	string str, begin, end;
//	str.clear();
//	str.append(p->data,p->size);
//
//	istringstream stream(str);
//
////	if (VERBOSE)
////		cout << "node::recv_work(): received work = " << str << endl;
//
//	cout << "numberOfInterval: " << p->numberOfInterval << endl;
//	for(int i=0; i<p->numberOfInterval; i++){
//		begin.clear(); end.clear();
//
//		stream >> begin;
//		stream >> end;
//
//		BigInteger b = easyStringToBI(begin);
//		BigInteger e = easyStringToBI(end);
//
//		work *tmp = new work(b,e, pbb);
//
//		//I want to check the interval I am receiving is not empty.
//		if(tmp->empty())
//			delete tmp;
//		else{
//			pbb->tr->intervalForTodo.push_back(tmp);
//		}
//	}
}

void Node::recvWorkInWorkStealingGPU3(packet_t* p, pbab* pbb){


	char *data = p->data;
	int sizeInBytes = p->size;
	int sizeInElement = sizeInBytes / (sizeof(int)*(MAX_NBJOBS + 5 + (MAX_NBJOBS + 1)));

	if (VERBOSE){
		cout << "recv work, sizeInBytes: " << sizeInBytes << endl;
		cout << "recv work, sizeInElement: " << sizeInElement << endl;
	}

	int startingBytes = 0;
	for(int i = 0; i< sizeInElement; i++){
		raw_bb_problem* pp = (raw_bb_problem*) malloc(sizeof(raw_bb_problem));

		memcpy(pp->permutation, data + startingBytes, sizeof(int) * MAX_NBJOBS);
		startingBytes +=  (sizeof(int) * MAX_NBJOBS);

		memcpy(&(pp->limite1), data + startingBytes, sizeof(int));
		startingBytes +=  sizeof(int);

		memcpy(&(pp->limite2), data + startingBytes, sizeof(int));
		startingBytes +=  sizeof(int);

		memcpy(&(pp->couts_somme), data + startingBytes, sizeof(int));
		startingBytes +=  sizeof(int);

		memcpy(&(pp->depth), data + startingBytes, sizeof(int));
		startingBytes +=  sizeof(int);

		memcpy(&(pp->begin_end), data + startingBytes, sizeof(int));
		startingBytes +=  sizeof(int);

		memcpy(pp->ranks, data + startingBytes, sizeof(int) * (MAX_NBJOBS + 1));
		startingBytes +=  (sizeof(int) * (MAX_NBJOBS + 1));

		pp->couts[0] = pp->couts_somme;	pp->couts[1] = 0;

		pbb->tr->insert(pp);
	}

	//cout << "recvWorkInWorkStealingGPU3, tree size: " << pbb->tr->size() << endl;
}

void Node::gpuRecvDecomposedProblems(packet_t* p, pbab* pbb){

	char *data = p->data;
	int sizeInBytes = p->size;
	int sizeInElement = sizeInBytes / sizeof(raw_bb_problem);

	cout << "gpuRecvDecomposedProblems, sizeInBytes: " << sizeInBytes << endl;
	cout << "gpuRecvDecomposedProblems, sizeInElements: " << sizeInElement << endl;

	problemPool* pp = new problemPool();
	pp->children_d = (raw_bb_problem*) malloc(sizeInBytes);
	memcpy(pp->children_d, data, sizeInBytes);

	pp->children_size = sizeInElement;

	pbb->gp->readyPoolToGPU.push_back(pp);
}

void Node::recvOptimalSolution(packet_t* p, pbab* pbb){
	string s;
	s.clear();
	s.append((const char *) p->data, p->size);
	istringstream iss(s);

	pbb->wsltns->readOptimalSolution(iss,*(pbb->wsltns));
}

void Node::recvBroadcastIpMsg(packet_t* p){
	child_t* temp_pool = NULL;

	int num_peers = (p->size) / (100 + sizeof(peerid_t) + sizeof(uint64_t));

	char* recv_nodes = (char*)malloc(p->size);
	memcpy(recv_nodes, p->data, p->size);

	int index = 0;
	for(int i=0; i<num_peers; i++){
		int new_peer_id;
		memcpy(&new_peer_id, (recv_nodes + index), sizeof(peerid_t));

		if(new_peer_id != this->my_address->id){


			child_t* new_node = (child_t*)malloc(sizeof(child_t));
			new_node->id = new_peer_id;

			memcpy(&(new_node->listening_port), (recv_nodes + index + sizeof(peerid_t)), sizeof(uint64_t));
			memcpy(new_node->host_machine, (recv_nodes + index + sizeof(peerid_t) + sizeof(uint64_t)), 100);

			if(current_number_of_peers_in_random_pool == 0){
				new_node->next_sibling = NULL;
				new_node->previous_sibling = NULL;
				temp_pool = new_node;
			}else{
				new_node->next_sibling = temp_pool;
				new_node->previous_sibling = NULL;
				temp_pool->previous_sibling = new_node;
				temp_pool = new_node;
			}
			current_number_of_peers_in_random_pool++;
		}

		index = index + sizeof(peerid_t) + sizeof(uint64_t) + 100;
	}
	free(recv_nodes);

	// convert linked list of random nodes to dynamic array for accessing quickly;
	if (this->current_number_of_peers_in_random_pool > 0){
		this->random_pool_of_nodes = (child_t*)malloc(this->current_number_of_peers_in_random_pool * sizeof(child_t));
		int pos = 0;
		while (temp_pool != NULL){
			memcpy(&this->random_pool_of_nodes[pos], temp_pool, sizeof(child_t));

			pos++;

			child_t *need_to_be_free = temp_pool;
			temp_pool = temp_pool->next_sibling;
			free(need_to_be_free);
		}
	}
}

child_t* Node::searchChild(peerid_t tmp_id){
	child_t * tmp_child= first_child;
	child_t * tmp;
	while(tmp_child!=NULL){
		//if tmp_child is our required child
		if (tmp_child->id == tmp_id)
			return tmp_child;
		//else we move to next child
		tmp=tmp_child->next_sibling;
		tmp_child=tmp;
	}

	return NULL;
}

void Node::broadcastRestartTerrminateVote(){
	child_t * tmp_child = first_child;
	child_t * tmp;
	while(tmp_child != NULL){
		tmp_child->termination_signal = 0;

		packet_t *p =(packet_t *)malloc(sizeof(packet_t));
		// fill the packet source data
		p->host_peer = my_address->id;
		p->host_port = my_address->listening_port;
		memcpy(p->host_machine, my_address->host_machine, 100);

		// fill the packet message type.
		p->size=0;					//packet contains no data
		p->message_type=18;

		// random destination
		p->dest_peer = tmp_child->id;
		p->dest_port = tmp_child->listening_port;
		memcpy(p->dest_machine, tmp_child->host_machine, 100);

		com->send_message(p);

		//we move to next child
		tmp = tmp_child->next_sibling;
		tmp_child = tmp;
	}
}

void Node::sendJoinRequest(){
	//cout<<"Sending join request to manager"<<endl;

	packet_t *p =(packet_t *)malloc(sizeof(packet_t));
	p->host_peer=my_address->id;
	p->dest_peer=-1;
	p->size=0;							//packet contains no data
	p->message_type=0; 					//msg typpe 0: means it is a join request to manager
	gethostname(p->host_machine, 100);	//host machine is MANAGER
	//gethostname(p->dest_machine, 100);//name of the machine where manager is running

	manager_machine.copy(p->dest_machine,100,0);
	p->host_port=com->PORT;
	p->dest_port=manager_port;

	com->send_message(p);
}

void Node::recvTerminationSignal(){
	jobFinished = true;
	broadcastTerminateSignalToGPUs();
	return;

}

void Node::printSlaveCPUs(){
	child_t* tmp_slave = first_slave;
	child_t* tmp = NULL;
	while (tmp_slave!= NULL){
		cout << "slave: " << tmp_slave->id << endl;

		tmp = tmp_slave->next_sibling;
		tmp_slave = tmp;
	}
}

void Node::printRandomPool(){
	for (int i = 0; i < this->current_number_of_peers_in_random_pool; i++){
		cout << "in pool, node " << this->random_pool_of_nodes[i].id << ", port " << this->random_pool_of_nodes[i].listening_port <<
				", machine "<< this->random_pool_of_nodes[i].host_machine << endl;
	}
}
