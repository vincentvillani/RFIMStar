/*
 * WorkerThread.cpp
 *
 *  Created on: 9 May 2016
 *      Author: vincentvillani
 */


#include "../Header/WorkerThread.h"

#include <math.h>
#include <string.h>

#ifdef BUILD_WITH_MKL
#include <mkl.h>
#include <mkl_trans.h>
#endif


//Private worker functions
//'pack' and 'unpack' are written by Ewan Barr
//https://github.com/ewanbarr/sigpyproc/blob/master/c_src/SigPyProc.c


/*Function to unpack 1,2 and 4 bit data
data is unpacked into an empty buffer
Note: Only unpacks big endian bit ordering*/
void unpack(unsigned char* inbuffer, float* outbuffer, int nbits, int nbytes)
{
  int ii,jj;

  switch(nbits)
  {

  case 1:

	for(ii = 0; ii < nbytes; ++ii)
	{
	  for(jj = 0; jj < 8; jj++)
	  {
		  outbuffer[(ii*8) + jj] = (inbuffer[ii] >> jj) & 1;
	  }
	}

	break;

  case 2:

    for(ii=0; ii < nbytes; ii++)
    {
		outbuffer[(ii*4) + 3] = inbuffer[ii] & LO2BITS;
		outbuffer[(ii*4) + 2] = (inbuffer[ii] & LOMED2BITS) >> 2;
		outbuffer[(ii*4) + 1] = (inbuffer[ii] & UPMED2BITS) >> 4;
		outbuffer[(ii*4) + 0 ] = (inbuffer[ii] & HI2BITS) >> 6;
    }

    break;

  case 4:

	for(ii=0;ii<nbytes;ii++)
	{
		outbuffer[(ii*2) + 1] = inbuffer[ii] & LO4BITS;
		outbuffer[(ii*2) + 0] = (inbuffer[ii] & HI4BITS) >> 4;
	}

	break;
  }
}



/*Function to pack bit data into an empty buffer*/
void pack(unsigned char* buffer, unsigned char* outbuffer, int nbits, int nbytes)
{
	int ii,pos;
	//int times = pow(nbits,2);
	int bitfact = 8/nbits;
	unsigned char val;

	switch(nbits)
	{

		case 1:
		for(ii=0;ii<nbytes/bitfact; ++ii)
		{

			pos = ii*8;

			val = (buffer[pos+7]<<7) |
				(buffer[pos+6]<<6) |
				(buffer[pos+5]<<5) |
				(buffer[pos+4]<<4) |
				(buffer[pos+3]<<3) |
				(buffer[pos+2]<<2) |
				(buffer[pos+1]<<1) |
				buffer[pos+0];

			outbuffer[ii] = val;

		}

		break;

		case 2:
		for(ii=0;ii<nbytes/bitfact; ++ii)
		{
			pos = ii*4;

			val = (buffer[pos]<<6) |
			(buffer[pos+1]<<4) |
			(buffer[pos+2]<<2) |
			buffer[pos+3];

			outbuffer[ii] = val;
		}

		break;

		case 4:

		for(ii = 0; ii < nbytes / bitfact; ++ii)
		{
			pos = ii*2;
			val = (buffer[pos]<<4) | buffer[pos+1];
			outbuffer[ii] = val;
		}

		break;

		}
}





void WorkerThreadMain(uint32_t workerThreadID, WorkerThreadData* threadData, MasterMailbox* masterMailbox,
		RFIMConfiguration* configuration)
{

	bool shouldShutdown = false;

	//Allocate space for the RFIM algorithm
	//h_valuesPerSample = number of beams (or filterbank files)
	//h_numberOfSamples = window size
	//batchSize = number of channels
	RFIMMemoryBlock* rfimMemoryBlock = new RFIMMemoryBlock(configuration->beamNum, configuration->windowSize, 1, configuration->channelNum);


	//Is there any work to do?
	//TODO: Figure out when to exit
	while(shouldShutdown == false)
	{
		//Lock the mutex
		std::unique_lock<std::mutex> lockGuard(threadData->workQueueMutex);


		//There is currently no work to do
		if(threadData->workQueue.size() == 0)
		{

			//std::cout << "Worker thread going to sleep..." << std::endl;

			//Go to sleep
			threadData->workQueueConditionVariable.wait(lockGuard, [&] { return threadData->workQueue.size(); } );

		}

		//std::cout << "Worker thread starting work..." << std::endl;

		//Get the current piece of work that needs to be done
		//At this point the mutex should be locked
		RawDataBlock* rawData = threadData->workQueue.front();

		//Remove it from the work queue and unlock the mutex
		threadData->workQueue.pop();
		lockGuard.unlock();


		//Unpack the data for use
		WorkerThreadUnpackData(workerThreadID, rawData, rfimMemoryBlock, configuration);



		//Multiplex the data
		WorkerThreadMultiplexData(rfimMemoryBlock, configuration);


		//Calculate the window size (AKA number of samples in RFIMStar jargon) of this iteration,
		//most of the time it will always be the same, unless we are at the end of the
		//filterbank files
		//This is so when we do RFIM we ignore samples that don't exist as we reach near the end of a file
		rfimMemoryBlock->h_numberOfSamples = (8 * (rawData->usedDataLength / configuration->beamNum)) /
				(configuration->channelNum * configuration->numberOfWorkerThreads * configuration->numBitsPerSample);


		//Run RFIM
		//TODO: ADD THIS. FOR NOW JUST DO A MEMCOPY OF THE DATA FROM INPUT TO OUTPUT
		//uint64_t totalSignalByteSize = sizeof(float) * rfimMemoryBlock->h_valuesPerSample * rfimMemoryBlock->h_numberOfSamples *
		//		rfimMemoryBlock->h_batchSize;

		//COPY ALL DATA OVER REGARDLESS IF IT IS USED OR NOT?
		uint64_t totalSignalByteSize = sizeof(float) * rfimMemoryBlock->h_valuesPerSample * configuration->windowSize *
					rfimMemoryBlock->h_batchSize;
		memcpy(rfimMemoryBlock->h_outputSignal, rfimMemoryBlock->h_inputSignal, totalSignalByteSize);


		//De-Multiplex the data
		WorkerThreadDeMultiplexData(rfimMemoryBlock, configuration);

		//Pack the data
		WorkerThreadPackData(workerThreadID, rawData, rfimMemoryBlock, configuration);

		//Is this the last block we should process?
		shouldShutdown = rawData->isLastBlock;


		//Pass to the writer thread
		masterMailbox->workerWriterMailbox->Worker_NotifyFinishedWithBlock(rawData);

	}


	std::cout << "Worker thread finishing..." << std::endl;

	delete rfimMemoryBlock;

}



void WorkerThreadUnpackData(uint32_t workerThreadID, RawDataBlock* rawDataBlock, RFIMMemoryBlock* rfimMemoryBlock,
		RFIMConfiguration* configuration)
{



	//Calculate the filterbank and thread offsets
	//uint64_t oneFilterbankByteSize = (rawDataBlock->usedDataLength / configuration->beamNum);
	uint64_t oneFilterbankByteSize = (rawDataBlock->totalDataLength / configuration->beamNum);
	uint64_t threadFilterbankByteSize = (oneFilterbankByteSize / configuration->numberOfWorkerThreads);
	uint64_t threadStartingOffset = (oneFilterbankByteSize / configuration->numberOfWorkerThreads) * workerThreadID;
	uint64_t outputOffset = (configuration->windowSize * rfimMemoryBlock->h_batchSize);
	//uint64_t outputOffset = (rfimMemoryBlock->h_numberOfSamples * rfimMemoryBlock->h_batchSize);

	//std::cout << "Number of Samples: " << rfimMemoryBlock->h_numberOfSamples << std::endl;
	//std::cout << "Values per Sample: " << rfimMemoryBlock->h_valuesPerSample << std::endl;
	//std::cout << "Batches: " << rfimMemoryBlock->h_batchSize << std::endl;
	//std::cout << "Output offset: " << outputOffset << std::endl;

	//std::cout << "Raw data used data length: " << rawDataBlock->usedDataLength << std::endl;
	//std::cout << "oneFilterbankByteSize byte size: " << oneFilterbankByteSize << std::endl;
	//std::cout << "threadFilterbankByteSize offset: " << threadFilterbankByteSize << std::endl;
	//std::cout << "threadStaringOffset: " << threadStartingOffset << std::endl;


	//unsigned char* testArray = new unsigned char[oneFilterbankByteSize * configuration->beamNum
	//											 * ( 8 / configuration->numBitsPerSample)];



	//For each filterbank beam, unpack the needed data for this worker thread
	for(uint32_t i = 0; i < configuration->beamNum; ++i)
	{
		//Unpack data from each filterbank file that we are going to use
		unpack(rawDataBlock->packedRawData + (i * oneFilterbankByteSize) + threadStartingOffset,
				rfimMemoryBlock->h_inputSignal + (outputOffset * i),
				configuration->numBitsPerSample, threadFilterbankByteSize);
	}

}




void WorkerThreadMultiplexData(RFIMMemoryBlock* rfimMemoryBlock, RFIMConfiguration* configuration)
{
	//Transpose the matrix in place to multiplex the signal correctly
	//

	#ifdef BUILD_WITH_MKL
	mkl_simatcopy('c', 't',
			rfimMemoryBlock->h_valuesPerSample, configuration->windowSize * rfimMemoryBlock->h_batchSize,
			1.0f, rfimMemoryBlock->h_inputSignal,
			configuration->windowSize * rfimMemoryBlock->h_batchSize,
			rfimMemoryBlock->h_valuesPerSample);
	#endif





}


void WorkerThreadDeMultiplexData(RFIMMemoryBlock* rfimMemoryBlock, RFIMConfiguration* configuration)
{
	//Transpose the matrix in place to demultiplex the signal correctly
	//

	#ifdef BUILD_WITH_MKL
	mkl_simatcopy('c', 't',
			configuration->windowSize * rfimMemoryBlock->h_batchSize, rfimMemoryBlock->h_valuesPerSample,
			1.0f, rfimMemoryBlock->h_outputSignal,
			rfimMemoryBlock->h_valuesPerSample,
			configuration->windowSize * rfimMemoryBlock->h_batchSize);
	#endif
}



void WorkerThreadPackData(uint32_t workerThreadID, RawDataBlock* rawDataBlock, RFIMMemoryBlock* rfimMemoryBlock,
		RFIMConfiguration* configuration)
{

	//use the input signal data as a buffer to convert the outputSignal floats into unsigned chars
	unsigned char* outputCharData = (unsigned char*)rfimMemoryBlock->h_inputSignal;

	//Interpret the output data as chars
	//TODO: add this back? rfimMemoryBlock->h_numberOfSamples instead of configuration->windowSize
	uint64_t totalSignalLength = rfimMemoryBlock->h_valuesPerSample * configuration->windowSize * rfimMemoryBlock->h_batchSize;
	for(uint64_t i = 0; i < totalSignalLength; ++i)
	{
		outputCharData[i] = (unsigned char)rfimMemoryBlock->h_outputSignal[i];
	}


	/*
	 * 	uint64_t oneFilterbankByteSize = (rawDataBlock->totalDataLength / configuration->beamNum);
		uint64_t threadFilterbankByteSize = (oneFilterbankByteSize / configuration->numberOfWorkerThreads);
		uint64_t threadStartingOffset = (oneFilterbankByteSize / configuration->numberOfWorkerThreads) * workerThreadID;
		uint64_t outputOffset = (configuration->windowSize * rfimMemoryBlock->h_batchSize);
	 *
	 */

	//rfimMemoryBlock->h_numberOfSamples instead of configuration->windowSize
	uint64_t outputCharDataOffset = configuration->windowSize * rfimMemoryBlock->h_batchSize;

	//rawDataBlock->usedDataLength instead of rawDataBlock->totalDataLength
	uint64_t oneFilterbankByteSize = (rawDataBlock->totalDataLength / configuration->beamNum);
	uint64_t threadFilterbankByteSize = (oneFilterbankByteSize / configuration->numberOfWorkerThreads);
	uint64_t threadStartingOffset = (oneFilterbankByteSize / configuration->numberOfWorkerThreads) * workerThreadID;


	//For each filterbank beam, pack the processed data
	for(uint32_t i = 0; i < configuration->beamNum; ++i)
	{

		//pack data back into the filterbank format required
		pack(outputCharData + (i * outputCharDataOffset),
				rawDataBlock->packedRawData + (i * oneFilterbankByteSize) + threadStartingOffset,
				configuration->numBitsPerSample, threadFilterbankByteSize);

	}

}



