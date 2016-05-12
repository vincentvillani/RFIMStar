/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "Header/RFIMStar.h"

#include "Header/UnitTests.h"

//TODO: THE WORKER THREADS SHOULD ONLY MULTIPLEX THERE OWN PORTION OF THE DATA!
//TODO: Add the multiplexing with MKL
//TODO: Write a unit test to test that the multiplexing with MKL actually works
//TODO: Actually add the RFIM routine
//TODO: Test the performance of the MKL in-place transpose with the out of place transpose
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
