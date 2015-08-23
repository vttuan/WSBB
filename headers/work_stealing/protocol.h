#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "types.h"
#include "../pbab.h"
#include "node.h"
#include "statistics.h"
//#include "../../headers/gpu.h"


class Protocol{
public:
	Protocol(int duration, int port, char* manager_machine, pbab* pbb);
	~Protocol();
	void error(const char*);
	pthread_t* run();
	Node* node;
	pbab* pbb;

private:
	pthread_t listening_thread;
};


#endif /* PROTOCOL_H_ */
