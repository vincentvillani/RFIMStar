/*
 * Main.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <functional>
#include <algorithm>

//#include "DeadSigproc/DeadSigproc.h"
#include "Header/RFIMConfiguration.h"

#include "Header/ReaderThread.h"
#include "Header/WorkerThread.h"


//TODO: Add the ability to read in and write out different nbit values
//TODO: Calculate how many dimensions to reduce at run-time, rather than a hardcoded 1
//TODO: Pass the message down from the reader thread that the last raw data block has been read and that after processing it the other threads can shut down
//TODO: Make the configuration be setup by passing arguments to the program via the command line
int main()
{

	//"/home/vincent/Documents/FilterbankFiles/";
	//"/lustre/projects/p002_swin/surveys/SUPERB/2016-01-05-12:07:06/";
	//"/Users/vincentvillani/Desktop/FilterbankFiles/2016-01-05-12:07:06/"
	std::string filenamePrefix = "/Users/vincentvillani/Desktop/FilterbankFiles/2016-01-05-12:07:06/";
	std::string filenamePostfix = "/2016-01-05-12:07:06.fil";
	std::stringstream ss;

	uint32_t workerThreads = 1;
	uint32_t windowSize = 15625;
	//uint32_t channelNum = 1024;
	uint32_t beamNum = 2;
	uint32_t numberOfRawDataBlocks = 100000;


	//Open the filterbanks
	std::vector<SigprocFilterbank*> filterbanks;

	for(uint32_t i = 1; i < beamNum + 1; ++i)
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
	RFIMConfiguration configuration(workerThreads, windowSize, (uint32_t)filterbanks[0]->get_nchans(),
			beamNum, numberOfRawDataBlocks, (uint32_t)filterbanks[0]->get_nbits());


	//Allocate raw data block memory
	std::vector<RawDataBlock*> rawDataBlockVector;
	uint64_t rawDataBlockArrayLength = (configuration.numberOfWorkerThreads * configuration.beamNum * configuration.windowSize *
			configuration.channelNum * configuration.numBitsPerSample) / 8;


	for(uint32_t i = 0; i < numberOfRawDataBlocks; ++i)
	{
		RawDataBlock* RDB = new RawDataBlock(i, rawDataBlockArrayLength, configuration.numBitsPerSample);
		rawDataBlockVector.push_back(RDB);
	}


	//Setup thread data objects and mailboxes etc

	//Reader and worker thread data objects
	ReaderThreadData* readerThreadData = new ReaderThreadData(&rawDataBlockVector);
	std::vector<WorkerThreadData*> workerThreadDataVector;
	for(uint32_t i = 0; i < configuration.numberOfWorkerThreads; ++i)
	{
		workerThreadDataVector.push_back(new WorkerThreadData());
	}

	//individual mailboxes
	ReaderWorkerMailbox* readerWorkerMailbox = new ReaderWorkerMailbox(readerThreadData, &workerThreadDataVector);

	//Master mailbox
	MasterMailbox* masterMailbox = new MasterMailbox(readerWorkerMailbox);

	//Start threads
	//Reading thread
	std::thread readingThread(ReaderThreadMain, std::ref(filterbanks), readerThreadData, &configuration, masterMailbox);

	//Worker threads
	std::vector<std::thread*> workerThreadVector;
	for(uint32_t i = 0; i < configuration.numberOfWorkerThreads; ++i)
	{
		workerThreadVector.push_back(new std::thread(WorkerThreadMain, i, workerThreadDataVector[i], masterMailbox, &configuration));
	}



	//Wait till should exit is set (join with all created threads?)
	readingThread.join();
	for(uint32_t i = 0; i < workerThreadVector.size(); ++i)
	{
		workerThreadVector[i]->join();
	}


	//Free all memory
	//-----------------------------

	//Filterbanks
	for(uint32_t i = 0; i < filterbanks.size(); ++i)
	{
		delete filterbanks[i];
	}

	//Raw data blocks
	for(uint32_t i = 0; i < rawDataBlockVector.size(); ++i)
	{
		delete rawDataBlockVector[i];
	}

	//Thread data and threads
	delete readerThreadData;
	for(uint32_t i = 0; i < configuration.numberOfWorkerThreads; ++i)
	{
		delete workerThreadDataVector[i]; //Worker data
		delete workerThreadVector[i]; //Worker std::threads
	}


	//Mailboxes
	delete readerWorkerMailbox;
	delete masterMailbox;


	return 0;
}
