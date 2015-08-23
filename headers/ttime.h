//========================================================================================================================================
using namespace std;
#include <time.h>
#include <iostream>
#include <stdlib.h>
//========================================================================================================================================
#ifndef TTIME_H
#define TTIME_H

#define WORKER_BALANCING 0
#define CHECKPOINT_TTIME 1

#define SIZE_TTIME       2

class ttime 
{public:
  time_t periods[SIZE_TTIME],lasts[SIZE_TTIME]; 
  ttime();
  static time_t time_get(); 
  void  wait(int index);
  void period_set(int index, time_t t); 
  bool period_passed(int index);
};
#endif





