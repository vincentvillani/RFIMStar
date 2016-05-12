/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "Header/RFIMStar.h"

#include "Header/UnitTests.h"

//TODO: Move the h_numberOfSamples calculation into the middle of the worker thread loop so it's more visible
//TODO: Write a unit test that tests that the packing/unpacking works with no RFIM being done and the filterbank files are unchanged
//TODO: Actually add the RFIM routine
//TODO: Add the ability to read in and write out different nbit values
//TODO: Calculate how many dimensions to reduce at run-time, rather than a hardcoded 1
//TODO: Make the configuration be setup by passing arguments to the program via the command line
int main()
{

	//Unit tests
	RunAllUniTests();

	//RFIMStarRoutine();

	return 0;
}
