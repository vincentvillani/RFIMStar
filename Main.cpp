/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "Header/RFIMStar.h"

#include "Header/UnitTests.h"


//TODO: Refractor out of place multiplexing code to make more sense
//TODO: Actually add the RFIM routine
//TODO: Calculate how many dimensions to reduce at run-time, rather than a hardcoded 1
//TODO: Make the configuration be setup by passing arguments to the program via the command line
//TODO: Add the ability to read in and write out different nbit values
int main()
{

	//Unit tests
	RunAllUniTests();

	//RFIMStarRoutine();

	return 0;
}
