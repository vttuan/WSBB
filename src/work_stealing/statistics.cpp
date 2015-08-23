using namespace std;
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <string.h>
#include "../../headers/work_stealing/statistics.h"

unsigned long statistics::num_work_request_msg_sent = 0;
unsigned long statistics::num_work_reply_msg_sent = 0;

unsigned long statistics::num_work_request_msg_recv = 0;
unsigned long statistics::num_work_reply_msg_recv = 0;

unsigned long statistics::total_num_received_msg = 0;
unsigned long statistics::total_num_sent_msg = 0;

unsigned long statistics::total_CPU_computing_time = 0;
unsigned long statistics::total_CPU_computing_time_after_GPU = 0;
unsigned long statistics::total_idle_time = 0;
unsigned long statistics::total_communication_time = 0;
unsigned long statistics::total_CPU_serving_time = 0;
unsigned long statistics::total_execution_time = 0;

unsigned long statistics::total_GPU_computing_time = 0;
unsigned long statistics::total_GPU_computing_time1 = 0;

unsigned long statistics::total_GPU_FOLD_time = 0;
unsigned long statistics::total_GPU_UNFOLD_time = 0;

unsigned long statistics::total_waiting_time_for_work = 0;

unsigned long statistics::time_get_in_second()
{
	time_t rawtime;
	time ( &rawtime );
    return rawtime;
}

unsigned long statistics::time_get_in_us(){
	unsigned long resutl;
	struct timeval t;
	gettimeofday(&t, NULL);
	resutl = 1000000;
	resutl *= t.tv_sec;
	resutl += t.tv_usec;
	return resutl;
}

void statistics::print(){
	cout <<	endl << endl << endl;

	cout << "***********MESSAGES***************" << endl;
	cout << "Statistics::num_work_request_msg, I sent: " << statistics::num_work_request_msg_sent << endl;
	cout << "Statistics::num_work_reply_msg, I got: " << statistics::num_work_reply_msg_recv << endl;

	cout << "Statistics::num_work_request_msg, I got: " << statistics::num_work_request_msg_recv << endl;
	cout << "Statistics::num_work_reply_msg, I sent: " << statistics::num_work_reply_msg_sent << endl << endl;

	cout << "Statistics::total_num_received_msg, I got: " << statistics::total_num_received_msg << endl;
	cout << "Statistics::total_num_sent_msg, I sent: " << statistics::total_num_sent_msg << endl;
	cout << "*********************************" << endl << endl;

	cout << "*************GPU*****************" << endl;
	cout << "Statistics::total_GPU_computing_time1: " << statistics::total_GPU_computing_time1 << endl;
	cout << "Statistics::total_GPU_computing_time: " << statistics::total_GPU_computing_time << endl;
	cout << "Statistics::total_GPU_FOLD_time: " << statistics::total_GPU_FOLD_time << endl;
	cout << "Statistics::total_GPU_UNFOLD_time: " << statistics::total_GPU_UNFOLD_time << endl;
	cout << "*********************************" << endl << endl;

	cout << "*************CPU*****************" << endl;
	cout << "Statistics::total_CPU_computing_time_before_GPU: " << statistics::total_CPU_computing_time<< endl;
	cout << "Statistics::total_CPU_computing_time_after_GPU: " << statistics::total_CPU_computing_time_after_GPU<< endl;

	cout << "Statistics::total_communication_time: " << statistics::total_communication_time << endl;
	cout << "Statistics::total_CPU_Serving_Time: " << statistics::total_CPU_serving_time << endl;

	cout << "Statistics::total_execution_time: " << statistics::total_execution_time << endl;
	cout << "*********************************" << endl << endl;
}
