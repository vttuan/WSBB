#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>

using namespace std;

int main()
{
	srand(time(NULL));
	cout<< (rand() % 50000) + 10000 <<endl;
}
