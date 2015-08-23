using namespace std;
#include <string>
#include <iostream>
#include <sstream>
#include "../int/BigInteger.hh"
#include "../int/BigIntegerUtils.hh"

#ifndef weightS_H
#define weightS_H
class pbab;
class weights {
public:
	BigInteger depths[500 + 1];
	weights(pbab* instance);
};
#endif
