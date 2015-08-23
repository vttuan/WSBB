using namespace std;
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "../headers/ttime.h"
#include "../headers/arguments.h"

ttime::ttime()
{
	period_set(CHECKPOINT_TTIME, arguments::checkpointv);
	period_set(WORKER_BALANCING, arguments::balancingv);
}

time_t ttime::time_get()
{
	time_t tmp;
	time(&tmp);
	return tmp;
}

void ttime::period_set(int index, time_t t)
{
	srand48(getpid());
	lasts[index] = (time_t) (time_get() - (t * 1.0) * drand48());
	periods[index] = t;
}

bool ttime::period_passed(int index)
{
	time_t tmp = time_get();
	if ((tmp - lasts[index]) < periods[index])
		return false;
	lasts[index] = tmp;
	return true;
}
