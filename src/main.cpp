using namespace std;

#include <iostream>
#include <getopt.h>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"
#include "../headers/ttime.h"
#include "../headers/decompose_abstract.h"
#include "../headers/decompose_beginend.h"
#include "../headers/weights.h"
#include "../headers/peer.h"
#include "../headers/solutions.h"
#include "../headers/eliminate_abstract.h"
#include "../headers/eliminate_pareto.h"
#include "../headers/tree.h"
#include "../headers/explorer.h"
#include "../headers/bound_abstract.h"
#include "../headers/bound_flowshop.h"
#include "../headers/instance_abstract.h"
#include "../headers/instance_flowshop.h"
#include "../headers/work.h"
#include "../headers/works.h"
#include "../headers/pbab.h"
#include "../headers/arguments.h"

#include "../headers/work_stealing/protocol.h"
#include <cutil.h>
#include <cuda.h>
#include <cuda_runtime.h>

int get_available_devices(int* nb_block, int* block_size)
{
	cudaDeviceProp  prop;

    int count;

    cudaGetDeviceCount( &count ) ;

    printf( "\n\n   --- Number of device %d ---\n\n", count );

    for (int i=0; i< count; i++)
    {
        cudaGetDeviceProperties( &prop, i ) ;
        printf( "   --- General Information for device %d ---\n\n", i );
        printf( "Name:  %s\n", prop.name );
        printf( "Compute capability:  %d.%d\n", prop.major, prop.minor );
        printf( "Clock rate:  %d\n", prop.clockRate );
        printf( "Total global mem:  %ul\n", prop.totalGlobalMem );
        printf( "Total constant Mem:  %d\n", prop.totalConstMem );
        printf( "Multiprocessor count:  %d\n", prop.multiProcessorCount );
        printf( "Shared mem per mp:  %d\n", prop.sharedMemPerBlock );
        printf( "Registers per mp:  %d\n", prop.regsPerBlock );
        printf( "Threads in warp:  %d\n", prop.warpSize );
        printf( "Max threads per block:  %d\n",prop.maxThreadsPerBlock );
        printf( "Max thread dimensions:  (%d, %d, %d)\n",prop.maxThreadsDim[0], prop.maxThreadsDim[1],prop.maxThreadsDim[2] );
        printf( "Max grid dimensions:  (%d, %d, %d)\n",prop.maxGridSize[0], prop.maxGridSize[1],prop.maxGridSize[2] );
        printf( "\n" );
    }

    *nb_block = prop.maxThreadsDim[0] / 2;
    *block_size = prop.maxThreadsPerBlock / 2;

    return count;
}

int main(int argc, char**argv)
{
	arguments::parse_arguments(argc, argv);

	int used_device = 0;
	int nb_block, block_size;

	#define YYY 50

	pthread_t* threads;
	pbab* pbabs;

	tree*tr = new tree(arguments::strategy);

	if(arguments::worker)
	{
		//THREADS CPU
		if (arguments::cpu_number == 1){
			arguments::type = 'c';
			std::cout << "i am a CPU worker my port is " << arguments::worker_port << endl;
			pbabs = new pbab(nb_block, block_size, used_device,
			new eliminate_pareto(),
			new decompose_beginend(),
			new instance_flowshop(arguments::instancev),
			new bound_flowshop(), tr);

		}else if (arguments::gpu_number == 1){
			arguments::type = 'g';

			used_device = arguments::devices;
			get_available_devices(&nb_block,&block_size);
			pbabs = new pbab(nb_block, block_size, used_device,
								new eliminate_pareto(),
								new decompose_beginend(),
								new instance_flowshop(arguments::instancev),
								new bound_flowshop(), tr);

			if (arguments::pool_size > 0)
				pbabs->gp->pool_size = arguments::pool_size;
			else
				pbabs->gp->pool_size = 8192;
		}

		cout << "Pool size: " << pbabs->gp->pool_size << endl;
		sleep(10);

		Protocol *protocol = new Protocol(3600, arguments::worker_port, arguments::index_name, pbabs);
		threads  = protocol->run();
		pthread_join(*threads, NULL);

		delete tr;
	}
	else if(arguments::root)
	{
		cout << "IN MASTER " <<endl<<flush;
		pbab *pbb = new pbab(nb_block, block_size, used_device,
							new eliminate_pareto(),
							NULL,//new decompose_beginend(),
							new instance_flowshop(arguments::instancev),
							new bound_flowshop(), tr);

	    cout << "I am a master my port is " << arguments::index_port<<" " << arguments::worker_port<<endl;

	    pbb->init();

	    sleep(99999999);
	}

	return 0;
}
