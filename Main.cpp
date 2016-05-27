/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include <iostream>

#include "Header/RFIMStar.h"
#include "Header/UnitTests.h"



//TODO: Generate a mask file for each block
//TODO: Calculate how many dimensions to reduce at run-time, rather than a hardcoded 1
//TODO: Make the configuration be setup by passing arguments to the program via the command line
//TODO: What if the values go above or below a the range that the bits can represent? (DO I FIND THE MIN AND MAX AND MAP IT?, OR JUST CLAMP?)
//TODO: Unit test for memory leaks
//TODO: Add the ability to read in and write out different nbit values
//TODO: Do a normalisation step before computing the covariance matrix, to see if there are any huge outliers, if there are, replace it with white noise or zeroes?
		//Median absolute deviation (MAD) to normalise
//TODO: insert signals into the actual filterbank files
int main()
{

	//Unit tests
	//RunAllUnitTests();



	uint32_t beamNum = 13;
	uint32_t rawDataBlockNum = 5;
	uint32_t numberOfWorkerThreads = 15;
	uint32_t windowSize = 169;
	uint32_t dimensionsToReduce = 1;

	std::string inputFilenamePrefix = "/lustre/projects/p002_swin/surveys/SUPERB/2014-12-06-04:36:26/";
	std::string inputFilenamePostfix = "/2014-12-06-04:36:26.fil";

	std::string outputFilenamePrefix = "/lustre/projects/p002_swin/vvillani/badObservation/";
	std::string outputFilenamePostfix = "zpna1-169W.fil";

	RFIMConfiguration configuration = RFIMConfiguration(numberOfWorkerThreads, windowSize, beamNum, dimensionsToReduce, rawDataBlockNum,
			inputFilenamePrefix, inputFilenamePostfix,
			outputFilenamePrefix, outputFilenamePostfix);


	RFIMStarRoutine(&configuration);

	std::cout << "Done!" << std::endl;







	return 0;
}
