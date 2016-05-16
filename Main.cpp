/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include <iostream>

#include "Header/RFIMStar.h"
#include "Header/UnitTests.h"


//TODO: Calculate how many dimensions to reduce at run-time, rather than a hardcoded 1
//TODO: Check for memory allocation errors
//TODO: Unit test for memory leaks
//TODO: Make the configuration be setup by passing arguments to the program via the command line
//TODO: Add the ability to read in and write out different nbit values
int main()
{

	//Unit tests
	RunAllUnitTests();


/*
	uint32_t beamNum = 13;
	uint32_t rawDataBlockNum = 3;
	uint32_t numberOfWorkerThreads = 10;
	uint32_t windowSize = 15625;
	uint32_t dimensionsToReduce = 2;

	std::string inputFilenamePrefix = "/lustre/projects/p002_swin/surveys/SUPERB/2016-01-05-12:07:06/";
	std::string inputFilenamePostfix = "/2016-01-05-12:07:06.fil";

	std::string outputFilenamePrefix = "/lustre/projects/p002_swin/vvillani/";
	std::string outputFilenamePostfix = "a2-15625W.fil";

	RFIMConfiguration configuration = RFIMConfiguration(numberOfWorkerThreads, windowSize, beamNum, dimensionsToReduce, rawDataBlockNum,
			inputFilenamePrefix, inputFilenamePostfix,
			outputFilenamePrefix, outputFilenamePostfix);


	RFIMStarRoutine(&configuration);

	std::cout << "Done!" << std::endl;

	*/


	return 0;
}
