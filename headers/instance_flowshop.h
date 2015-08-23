using namespace std;

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

#ifndef INSTANCE_FLOWSHOP_H
#define INSTANCE_FLOWSHOP_H

struct instance_flowshop :public instance_abstract
{instance_flowshop(int id);
 int get_job_number(int id);
 int get_machine_number(int id);
 long unif(long *seed, long low, long high);
 void generate_instance(int id, ostream& stream);
};

#endif

