//=============================================================================================================
#ifndef _STATISTICS_
#define _STATISTICS_

using namespace std;
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <string.h>

class statistics {
public:

	static unsigned long time_get_in_second();
	static unsigned long time_get_in_us();

	static void print();

	static unsigned long num_work_request_msg_sent;
	static unsigned long num_work_reply_msg_sent;

	static unsigned long num_work_request_msg_recv;
	static unsigned long num_work_reply_msg_recv;

	static unsigned long total_num_received_msg;
	static unsigned long total_num_sent_msg;

	static unsigned long total_CPU_computing_time;
	static unsigned long total_CPU_computing_time_after_GPU;

	static unsigned long total_idle_time;
	static unsigned long total_communication_time;
	static unsigned long total_CPU_serving_time;
	static unsigned long total_execution_time;

	static unsigned long total_GPU_computing_time;
	static unsigned long total_GPU_computing_time1;
	static unsigned long total_GPU_FOLD_time;
	static unsigned long total_GPU_UNFOLD_time;

	static unsigned long total_waiting_time_for_work;
};
#endif

