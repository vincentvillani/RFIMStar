/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "Header/RFIMStar.h"

#include "Header/UnitTests.h"


//TODO: EIGENVALUES/VECTORS ARE RETURNED FROM LOWEST TO HIGHEST! ACCOUNT FOR THIS!
//TODO: Actually add the RFIM routine
//TODO: Write unit tests for the different RFIM steps
//TODO: Calculate how many dimensions to reduce at run-time, rather than a hardcoded 1
//TODO: Check for memory allocation errors
//TODO: Unit test for memory leaks
//TODO: Make the configuration be setup by passing arguments to the program via the command line
//TODO: Add the ability to read in and write out different nbit values
int main()
{

	//Unit tests
	RunAllUniTests();

	//RFIMStarRoutine();

	return 0;
}
