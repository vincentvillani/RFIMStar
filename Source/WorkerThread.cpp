/*
 * WorkerThread.cpp
 *
 *  Created on: 9 May 2016
 *      Author: vincentvillani
 */


#include "../Header/WorkerThread.h"

#include <math.h>
#include <string.h>
#include <algorithm>

#include "../Header/RFIMHelperFunctions.h"

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
		for(ii=0;ii<nbytes/bitfact; ++ii) //for(ii=0;ii<nbytes/bitfact; ++ii)
		{
			//printf("Running\n");
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




void RFIM(RFIMMemoryBlock* rfimMemoryBlock, RFIMConfiguration* RFIMConfiguration)
{
	CalculateCovarianceMatrix(rfimMemoryBlock);

	EigenvalueSolver(rfimMemoryBlock);

	EigenReductionAndFiltering(rfimMemoryBlock, RFIMConfiguration);
}




void WorkerThreadMain(uint32_t workerThreadID, WorkerThreadData* threadData, MasterMailbox* masterMailbox,
		RFIMConfiguration* configuration)
{

	bool shouldShutdown = false;

	//Allocate space for the RFIM algorithm
	//h_valuesPerSample = number of beams (or filterbank files)
	//h_numberOfSamples = window size
	//batchSize = number of channels
	RFIMMemoryBlock* rfimMemoryBlock = new RFIMMemoryBlock(configuration->beamNum, configuration->windowSize, configuration->dimensionsToReduce, configuration->channelNum,
			configuration->generatingMask);


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

		//Check if there is any actual work to do?
		//Calculate the total number of samples that the block contains (this can change if it is near the end of the file)
		//Calculate if there is any work for this worker thread to do
		uint64_t totalSamplesInRawDataBlock = (rawData->usedDataLength * 8) /
				(configuration->channelNum * configuration->beamNum * configuration->numBitsPerSample);
		uint64_t threadSampleOffset = workerThreadID * configuration->windowSize;
		uint64_t sampleDifference = totalSamplesInRawDataBlock - threadSampleOffset;

		//If it is greater than zero, then there is work to be done
		//Calculate how much should be done
		if(sampleDifference > 0)
		{
			//Make sure the samples are bounded by the max window size
			rfimMemoryBlock->h_numberOfSamples = std::min((uint64_t)configuration->windowSize, sampleDifference);

			//printf("threadID: %llu\n", workerThreadID);
			//printf("numberOfSamples: %llu\n", rfimMemoryBlock->h_numberOfSamples);
			//printf("usedDataLength: %llu\n\n", rawData->usedDataLength);
		}
		else //There is no work to do! Let the writer thread know your done
		{
			//printf("threadID: %llu\n", workerThreadID);
			//printf("numberOfSamples: %llu\n", 0);
			//printf("usedDataLength: %llu\n\n", rawData->usedDataLength);

			//Is this the last block we should process?
			shouldShutdown = rawData->isLastBlock;

			//Pass to the writer thread
			masterMailbox->workerWriterMailbox->Worker_NotifyFinishedWithBlock(rawData);

			break; //Skip everything else in this loop and shutdown
		}


		//Unpack the data for use
		WorkerThreadUnpackData(workerThreadID, rawData, rfimMemoryBlock, configuration);



		//Multiplex the data
		WorkerThreadMultiplexData(rfimMemoryBlock, configuration);


		//Normalise the data's underlying distribution
		//WorkerThreadNormaliseDistribution(rfimMemoryBlock);



		//Run RFIM
		RFIM(rfimMemoryBlock, configuration);



		//De-Multiplex the data
		WorkerThreadDeMultiplexData(rfimMemoryBlock, configuration);

		//Pack the data
		WorkerThreadPackData(workerThreadID, rawData, rfimMemoryBlock, configuration);

		//Is this the last block we should process?
		shouldShutdown = rawData->isLastBlock;


		//Pass to the writer thread
		masterMailbox->workerWriterMailbox->Worker_NotifyFinishedWithBlock(rawData);

	}


	//std::cout << "Worker thread finishing..." << std::endl;

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
				rfimMemoryBlock->h_outputSignal + (outputOffset * i),
				configuration->numBitsPerSample, threadFilterbankByteSize);
	}

}




void WorkerThreadMultiplexData(RFIMMemoryBlock* rfimMemoryBlock, RFIMConfiguration* configuration)
{

	//Offsets for the input signal
	uint64_t inputFreqChannelOffset = configuration->beamNum * configuration->windowSize;
	uint64_t inputTimeSampleOffset = configuration->beamNum;

	//Offsets for the output signal
    uint64_t outputBeamOffset = configuration->channelNum * configuration->windowSize;


	//For each frequency channel
	for(uint64_t f = 0; f < configuration->channelNum; ++f)
	{
		//The current frequency channel offset
		uint64_t currentInputFreqChannelOffset = f * inputFreqChannelOffset;

		//For each time sample
		for(uint64_t t = 0; t < configuration->windowSize; ++t)
		{
			uint64_t currentInputTimeSampleOffset = t * inputTimeSampleOffset;

			//For each beam
			for(uint64_t b = 0; b < configuration->beamNum; ++b)
			{
				//printf("inputSignalIndex: %llu\n", currentInputFreqChannelOffset + currentInputTimeSampleOffset + b);
				//printf("outputSignalIndex: %llu\n\n", (b * outputBeamOffset) + currentInputTimeSampleOffset + f);

				rfimMemoryBlock->h_inputSignal[currentInputFreqChannelOffset + currentInputTimeSampleOffset + b] =
					rfimMemoryBlock->h_outputSignal[(b * (outputBeamOffset)) +
													t * configuration->channelNum + f];

			}
		}

	}


	/*
	#ifdef BUILD_WITH_MKL
	mkl_somatcopy('c', 't',
			configuration->windowSize * rfimMemoryBlock->h_batchSize, rfimMemoryBlock->h_valuesPerSample,
			1.0f, rfimMemoryBlock->h_outputSignal, configuration->windowSize * rfimMemoryBlock->h_batchSize,
			rfimMemoryBlock->h_inputSignal, rfimMemoryBlock->h_valuesPerSample);
	#endif
	*/





}




void WorkerThreadNormaliseDistribution(RFIMMemoryBlock* rfimMemoryBlock)
{
	uint64_t multiplexedSignalLength = rfimMemoryBlock->h_valuesPerSample * rfimMemoryBlock->h_numberOfSamples;

	//For each freq channel
	for(uint64_t i = 0; i < rfimMemoryBlock->h_batchSize; ++i)
	{
		//Calculate the mean and std dev
		float* currentDataSet = rfimMemoryBlock->h_inputSignal + (i * rfimMemoryBlock->h_inputSignalBatchOffset);
		float mean = CalculateMean(currentDataSet, multiplexedSignalLength);
		float stdDev = CalculateStandardDeviation(currentDataSet, multiplexedSignalLength, mean);

		//For each sample in a channel
		//Compute normalise its stats
		for(uint64_t j = 0; j < multiplexedSignalLength; ++j)
		{
			currentDataSet[j] = (currentDataSet[j] - mean) / stdDev;
		}
	}

}




void WorkerThreadDeMultiplexData(RFIMMemoryBlock* rfimMemoryBlock, RFIMConfiguration* configuration)
{

	//Offsets for the input signal
	uint64_t inputFreqChannelOffset = configuration->beamNum * configuration->windowSize;
	uint64_t inputTimeSampleOffset = configuration->beamNum;

	//Offsets for the output signal
    uint64_t outputBeamOffset = configuration->channelNum * configuration->windowSize;


	//For each frequency channel
	for(uint64_t f = 0; f < configuration->channelNum; ++f)
	{
		//The current frequency channel offset
		uint64_t currentInputFreqChannelOffset = f * inputFreqChannelOffset;

		//For each time sample
		for(uint64_t t = 0; t < configuration->windowSize; ++t)
		{
			uint64_t currentInputTimeSampleOffset = t * inputTimeSampleOffset;

			//For each beam
			for(uint64_t b = 0; b < configuration->beamNum; ++b)
			{
				//printf("inputSignalIndex: %llu\n", currentInputFreqChannelOffset + currentInputTimeSampleOffset + b);
				//printf("outputSignalIndex: %llu\n\n", (b * outputBeamOffset) + currentInputTimeSampleOffset + f);

				rfimMemoryBlock->h_outputSignal[(b * (outputBeamOffset)) + t * configuration->channelNum + f] =
						rfimMemoryBlock->h_inputSignal[currentInputFreqChannelOffset + currentInputTimeSampleOffset + b];


			}
		}

	}







	/*
	#ifdef BUILD_WITH_MKL
	mkl_somatcopy('c', 't',
			rfimMemoryBlock->h_valuesPerSample, configuration->windowSize * rfimMemoryBlock->h_batchSize,
			1.0f, rfimMemoryBlock->h_inputSignal, rfimMemoryBlock->h_valuesPerSample,
			rfimMemoryBlock->h_outputSignal, configuration->windowSize * rfimMemoryBlock->h_batchSize);
	#endif
	*/

}



void WorkerThreadPackData(uint32_t workerThreadID, RawDataBlock* rawDataBlock, RFIMMemoryBlock* rfimMemoryBlock,
		RFIMConfiguration* configuration)
{

	//use the input signal data as a buffer to convert the outputSignal floats into unsigned chars
	unsigned char* outputCharData = (unsigned char*)rfimMemoryBlock->h_inputSignal;

	//Interpret the output data as chars
	//TODO: add this back? rfimMemoryBlock->h_numberOfSamples instead of configuration->windowSize
	uint64_t totalSignalLength = rfimMemoryBlock->h_valuesPerSample * configuration->windowSize * rfimMemoryBlock->h_batchSize;
	float maxValue = powf(2, configuration->numBitsPerSample) - 1.0f;


	for(uint64_t i = 0; i < totalSignalLength; ++i)
	{
		//MAKE SURE IT'S ROUNDED UP OR DOWN APPROPRIATELY
		//MAKE SURE IT'S WITHIN THE RANGE OF THE NBITS
		//BOUND IT IF IT'S NOT?
		//(std::min(std::max(value, 0.0f), maxValue) + 0.5f);
		outputCharData[i] =  (std::min( std::max( (rfimMemoryBlock->h_outputSignal[i] + 0.5f), 0.0f ), maxValue));
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
	//uint64_t threadFilterbankByteSize = (oneFilterbankByteSize / configuration->numberOfWorkerThreads);
	uint64_t threadStartingOffset = (oneFilterbankByteSize / configuration->numberOfWorkerThreads) * workerThreadID;

	/*
	printf("workerThreadID: %llu\n", workerThreadID);
	printf("TotalSignalLength: %llu\n", totalSignalLength);
	printf("outputCharDataOffset: %llu\n", outputCharDataOffset);
	printf("oneFilterbankByteSize: %llu\n", oneFilterbankByteSize);
	printf("threadFilterbankByteSize: %llu\n", threadFilterbankByteSize);
	printf("threadStartingOffset: %llu\n", threadStartingOffset);
	printf("configuration->beamNum: %lu\n", configuration->beamNum);
	printf("configuration->numBitsPerSample: %lu\n\n", configuration->numBitsPerSample);
	*/


	//For each filterbank beam, pack the processed data
	for(uint32_t i = 0; i < configuration->beamNum; ++i)
	{

		//pack data back into the filterbank format required
		pack(outputCharData + (i * outputCharDataOffset),
				rawDataBlock->packedRawData + (i * oneFilterbankByteSize) + threadStartingOffset,
				configuration->numBitsPerSample, outputCharDataOffset);

	}


	//If we are generating a mask, pack the values into the appropriate place in the raw data block
	if(rfimMemoryBlock->h_generatingMask)
	{

		//Pack the h_maskValues into 1 bit data
		pack(rfimMemoryBlock->h_maskValues,
				rawDataBlock->packedMaskData + (rfimMemoryBlock->h_maskValuesLength / 8) * workerThreadID,
				1, rfimMemoryBlock->h_maskValuesLength);

		//Set all the data to zero, so the next work this thread does has a clean set of mask values
		memset(rfimMemoryBlock->h_maskValues, 0, sizeof(unsigned char) * rfimMemoryBlock->h_maskValuesLength);
	}

}



