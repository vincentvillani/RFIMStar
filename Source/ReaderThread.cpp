/*
 * ReaderThread.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "../Header/ReaderThread.h"

#include <stdint.h>
#include <vector>




//Filterbanks are already open and ready to go
void ReaderThreadMain(std::vector<SigprocFilterbank*>& filterbankVector, ReaderThreadData* RTD, RFIMConfiguration* RFIMConfig)
{


	uint64_t* bytesReadPerBeam = new uint64_t[RFIMConfig->beamNum];

	//Figure out how many bytes we should read per filterbank each iteration
	uint64_t bytesToReadPerFilterbank = (RFIMConfig->windowSize * RFIMConfig->channelNum * RFIMConfig->batchSize *
			RFIMConfig->numberOfWorkerThreads * RFIMConfig->numBitsPerSample) / 8;

	//std::cout << "Bytes to read per beam: " << bytesToReadPerFilterbank << std::endl;



	//for all the other filterbank files
	while(filterbankVector[0]->hasReachedEOF() == false)
	{

		//std::cout << "queue size: " << RTD->rawDataBlockQueue->size() << std::endl;

		//Aquire the lock
		std::unique_lock<std::mutex> readerThreadLockGuard(*RTD->rawDataBlockQueueMutex);

		//Do we have to wait or can we start work right away?
		if(RTD->rawDataBlockQueue->size() == 0)
		{
			std::cout << "Reader thread is waiting for work..." << std::endl;

			//We have to wait for a buffer to use

			//This checks the predicate, if it is false it releases the lock and puts the thread to sleep
			//Once someone has alerted the condition variable the lock is aquired again and the predicate is checked again
			//If it is true it will keep the lock aquired
			RTD->rawDataBlockQueueCV->wait(readerThreadLockGuard, [&]{return RTD->rawDataBlockQueue->size();});


		}

		//std::cout << "We have work to do!" << std::endl;

		//Once we wake and the predicate is satifised we should have the lock again.

		//Get a reference to the buffer and then remove it from the queue
		RawDataBlock* currentBuffer =  *(RTD->rawDataBlockQueue->end() - 1);
		RTD->rawDataBlockQueue->erase( RTD->rawDataBlockQueue->end() - 1);



		//At this point we should have a buffer, so release the mutex
		readerThreadLockGuard.unlock();



		//Read in filterbank data
		for(uint32_t i = 0; i < filterbankVector.size(); ++i)
		{
			ReadFilterbankData(filterbankVector[i], currentBuffer->packedRawData + (i * bytesToReadPerFilterbank), bytesToReadPerFilterbank,
					bytesReadPerBeam + i);

			//std::cout << "Read in filterbank data: " << i << ", bytes read " << bytesReadPerBeam[i] << std::endl;
		}

		//std::cout << "Done reading in filterbank data!" << std::endl;



		//find the lowest number of bytes read
		uint64_t lowestBytes = bytesReadPerBeam[0];
		for(uint64_t i = 1; i < RFIMConfig->beamNum; ++i)
		{
			if(bytesReadPerBeam[i] < lowestBytes)
				lowestBytes = bytesReadPerBeam[i];
		}

		//Tell the worker threads to only process the lowest number of bytes per filterbank
		currentBuffer->usedDataLength = lowestBytes * RFIMConfig->beamNum;



		//TODO: Send it off to the worker threads

	}



	//Free memory allocated by this thread
	delete [] bytesReadPerBeam;

	std::cout << "Reader thread finished" << std::endl;

}
