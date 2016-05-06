/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include <string>
#include <sstream>
#include <vector>

#include "DeadSigproc/DeadSigproc.h"
#include "Header/RFIMConfiguration.h"


//TODO: Make the configuration be setup by passing arguments to the program via the command line
int main()
{

	std::string filenamePrefix = "/lustre/projects/p002_swin/surveys/SUPERB/2016-01-05-12:07:06/";
	std::string filenamePostfix = "/2016-01-05-12:07:06.fil";
	std::stringstream ss;

	uint32_t workerThreads = 6;
	uint32_t windowSize = 15625;
	uint32_t channelNum = 1024;
	uint32_t batchSize = 5;
	uint32_t beamNum = 13;
	uint32_t numberOfRawDataBlocks = 5;


	//Open the filterbanks
	std::vector<SigprocFilterbank*> filterbanks;

	for(uint32_t i = 0; i < beamNum; ++i)
	{
		ss << filenamePrefix;

		if(i < 10)
			ss << "0" << i;
		else
			ss << i;

		ss << filenamePostfix;

		//Open the filterbank file
		SigprocFilterbank* filterbankFile = new SigprocFilterbank(ss.str());

		//Add it to the vector
		filterbanks.push_back(filterbankFile);

		//Reset the string stream for the next iteration
		ss.str("");

	}


	//Setup configuration object
	RFIMConfiguration configuration(workerThreads, windowSize, (uint32_t)filterbanks[0]->get_nchans(), batchSize,
			beamNum, numberOfRawDataBlocks, (uint32_t)filterbanks[0]->get_nbits());

	//Allocate raw data block memory
	std::vector<RawDataBlock*> rawDataBlockVector;
	uint64_t rawDataBlockArrayLength = (workerThreads * windowSize * beamNum * batchSize * configuration.numBitsPerSample) / 8;

	for(uint32_t i = 0; i < numberOfRawDataBlocks; ++i)
	{
		RawDataBlock* RDB = new RawDataBlock(rawDataBlockArrayLength, configuration.numBitsPerSample);
		rawDataBlockVector.push_back(RDB);
	}

	//Setup thread data objects and mailboxes etc
	ReaderThreadData readerThreadData;
	readerThreadData.rawDataBlockQueue = rawDataBlockVector;



	//Start threads

	//Wait till should exit is set (join with all created threads?)

	//Free all memory

	//Raw data blocks
	for(uint32_t i = 0; i < rawDataBlockVector.size(); ++i)
	{
		delete rawDataBlockVector[i];
	}

	//Filterbanks
	for(uint32_t i = 0; i < filterbanks.size(); ++i)
	{
		delete filterbanks[i];
	}


	return 0;
}
