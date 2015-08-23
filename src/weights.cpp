using namespace std;
#include <string>
#include <iostream>
#include <sstream>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"

#include "../headers/pbab.h"
#include "../headers/weights.h"

weights::weights(pbab* pbb)
{
	depths[pbb->size] = 1;

	depths[pbb->size-1] = 1;

	for(int i=pbb->size-2, j=2 ;i>=0;i--, j++)
    {
		depths[i] = depths[i+1];
		depths[i] *= j;
    }
}


