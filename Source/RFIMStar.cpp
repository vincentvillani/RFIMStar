/*
 * RFIMStar.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */


#include "../Header/RFIMStar.h"


void RFIMStarRoutine(RFIMConfiguration* configuration)
{

	//"/home/vincent/Documents/FilterbankFiles/";
	//"/lustre/projects/p002_swin/surveys/SUPERB/2016-01-05-12:07:06/";
	//"/Users/vincentvillani/Desktop/FilterbankFiles/2016-01-05-12:07:06/"
	//std::string filenamePrefix = "/Users/vincentvillani/Desktop/FilterbankFiles/2016-01-05-12:07:06/";
	//std::string filenamePostfix = "/2016-01-05-12:07:06.fil";
	std::stringstream ssInputFilterbankFile;
	std::stringstream ssOutputFilterbankFile;



	//Open the filterbanks
	std::vector<SigprocFilterbank*> filterbanks;
	std::vector<SigprocFilterbankOutput*> outputFilterbanks;

	for(uint32_t i = 1; i < configuration->beamNum + 1; ++i)
	{
		ssInputFilterbankFile << configuration->inputFilenamePrefix;
		ssOutputFilterbankFile << configuration->outputFilenamePrefix;

		if(i < 10)
		{
			ssInputFilterbankFile << "0" << i;
			ssOutputFilterbankFile << "0" << i;
		}
		else
		{
			ssInputFilterbankFile << i;
			ssOutputFilterbankFile << i;
		}

		ssInputFilterbankFile << configuration->inputFilenamePostfix;
		ssOutputFilterbankFile << configuration->outputFilenamePostfix;

		//Open the input and output filterbank file
		SigprocFilterbank* filterbankFile = new SigprocFilterbank(ssInputFilterbankFile.str());
		SigprocFilterbankOutput* outputFilterbankFile = new SigprocFilterbankOutput(ssInputFilterbankFile.str(), ssOutputFilterbankFile.str());

		//Add it to the vectors
		filterbanks.push_back(filterbankFile);
		outputFilterbanks.push_back(outputFilterbankFile);

		//Reset the string streams for the next iteration
		ssInputFilterbankFile.str("");
		ssOutputFilterbankFile.str("");

	}


	//Set the filterbank variables on the RFIMConfiguration
	//Assume there is always at least one filterbank file
	//This is should be checked prior to even starting this routine
	configuration->setFilterbankVariables(filterbanks[0]);




	//Allocate raw data block memory
	std::vector<RawDataBlock*> rawDataBlockVector;
	uint64_t rawDataBlockArrayLength = (configuration->numberOfWorkerThreads * configuration->beamNum * configuration->windowSize *
			configuration->channelNum * configuration->numBitsPerSample) / 8;


	for(uint32_t i = 0; i < configuration->rawDataBlockNum; ++i)
	{
		RawDataBlock* RDB = new RawDataBlock(i, rawDataBlockArrayLength, configuration->numBitsPerSample);
		rawDataBlockVector.push_back(RDB);
	}


	//Setup thread data objects and mailboxes etc

	//Reader and worker thread data objects
	ReaderThreadData* readerThreadData = new ReaderThreadData(&rawDataBlockVector);

	std::vector<WorkerThreadData*> workerThreadDataVector;
	for(uint32_t i = 0; i < configuration->numberOfWorkerThreads; ++i)
	{
		workerThreadDataVector.push_back(new WorkerThreadData());
	}

	WriterThreadData* writerThreadData = new WriterThreadData(outputFilterbanks);


	//individual mailboxes
	ReaderWorkerMailbox* readerWorkerMailbox = new ReaderWorkerMailbox(readerThreadData, &workerThreadDataVector);
	WorkerWriterMailbox* workerWriterMailbox = new WorkerWriterMailbox(writerThreadData, configuration);
	WriterReaderMailbox* writerReaderMailbox = new WriterReaderMailbox(readerThreadData);

	//Master mailbox
	MasterMailbox* masterMailbox = new MasterMailbox(readerWorkerMailbox, workerWriterMailbox, writerReaderMailbox);


	//Start threads
	//Reading thread
	std::thread readingThread(ReaderThreadMain, std::ref(filterbanks), readerThreadData, configuration, masterMailbox);

	//Worker threads
	std::vector<std::thread*> workerThreadVector;
	for(uint32_t i = 0; i < configuration->numberOfWorkerThreads; ++i)
	{
		workerThreadVector.push_back(new std::thread(WorkerThreadMain, i, workerThreadDataVector[i], masterMailbox, configuration));
	}

	//Start the writer thread
	std::thread writerThread(WriterThreadMain, writerThreadData, configuration, masterMailbox);

	//Wait till should exit is set (join with all created threads?)
	readingThread.join();

	for(uint32_t i = 0; i < workerThreadVector.size(); ++i)
	{
		workerThreadVector[i]->join();
	}

	writerThread.join();



	//Done processing everything at this point




	//Free all memory
	//-----------------------------

	//Filterbanks
	for(uint32_t i = 0; i < filterbanks.size(); ++i)
	{
		delete filterbanks[i];
		delete outputFilterbanks[i];
	}

	//Raw data blocks
	for(uint32_t i = 0; i < rawDataBlockVector.size(); ++i)
	{
		delete rawDataBlockVector[i];
	}

	//Thread data and threads
	delete readerThreadData;
	for(uint32_t i = 0; i < configuration->numberOfWorkerThreads; ++i)
	{
		delete workerThreadDataVector[i]; //Worker data
		delete workerThreadVector[i]; //Worker std::threads
	}
	delete writerThreadData;


	//Mailboxes
	delete readerWorkerMailbox;
	delete workerWriterMailbox;
	delete writerReaderMailbox;
	delete masterMailbox;

}

