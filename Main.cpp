/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include <iostream>

#include "Header/RFIMStar.h"
#include "Header/UnitTests.h"


//TODO: The variable removal of dimensions currently sucks. Find a way to make it better.
//TODO: Is it valid to add the variances here? The variances of each beam should be independent, right?
//TODO: Make the configuration be setup by passing arguments to the program via the command line
//TODO: What if the values go above or below a the range that the bits can represent? (DO I FIND THE MIN AND MAX AND MAP IT?, OR JUST CLAMP?)
//TODO: Unit test for memory leaks
//TODO: Add the ability to read in and write out different nbit values
//TODO: Do a normalisation step before computing the covariance matrix, to see if there are any huge outliers, if there are, replace it with white noise or zeroes?
		//Median absolute deviation (MAD) to normalise. Don't actually normalise the data, only use it to identify samples to remove
//TODO: insert signals into the actual filterbank files
int main()
{

	//Unit tests
	//RunAllUnitTests();



	uint32_t beamNum = 13;
	uint32_t rawDataBlockNum = 3;
	uint32_t numberOfWorkerThreads = 15;
	uint32_t windowSize = 15625;
	uint32_t dimensionsToReduce = 1;

	std::string inputFilenamePrefix = "/lustre/projects/p002_swin/surveys/SUPERB/2016-01-05-12:07:06/";
	std::string inputFilenamePostfix = "/2016-01-05-12:07:06.fil";

	std::string outputFilenamePrefix = "/lustre/projects/p002_swin/vvillani/";
	std::string outputFilenamePostfix = "pnva1-15625W.fil";

	RFIMConfiguration configuration = RFIMConfiguration(numberOfWorkerThreads, windowSize, beamNum, dimensionsToReduce, rawDataBlockNum,
			inputFilenamePrefix, inputFilenamePostfix,
			outputFilenamePrefix, outputFilenamePostfix);


	RFIMStarRoutine(&configuration);

	std::cout << "Done!" << std::endl;







	return 0;
}
