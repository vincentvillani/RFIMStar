/*
 * WriterThread.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */


#include "../Header/WriterThread.h"


RawDataBlock* findWork(WriterThreadData* writerThreadData, RFIMConfiguration* configuration, uint64_t currentBlockID)
{
	RawDataBlock* rawDataBlock = NULL;

	//Go through the work vector and see if there is anything to do
	for(uint32_t i = 0; i < writerThreadData->writeDataQueue.size(); ++i)
	{
		//Is this the next block that should be written AND all threads have finished processing it?
		if( currentBlockID == writerThreadData->writeDataQueue[i]->rawDataBlockID &&
				writerThreadData->writeDataQueue[i]->workerThreadsCompletedProcessing == configuration->numberOfWorkerThreads)
		{
			//Get a reference to the current block to write & remove it from the workQueue
			rawDataBlock = writerThreadData->writeDataQueue[i];
			writerThreadData->writeDataQueue.erase(writerThreadData->writeDataQueue.begin() + i);
			break;
		}
	}

	return rawDataBlock;
}


void WriterThreadMain(WriterThreadData* writerThreadData, RFIMConfiguration* configuration, MasterMailbox* masterMailbox)
{
	bool shouldShutdown = false;
	uint64_t currentBlockID = 0;


	while(shouldShutdown == false)
	{
		//Is there anything to do?
		//Lock the mutex
		std::unique_lock<std::mutex> workQueueLock(writerThreadData->writeDataQueueMutex);


		RawDataBlock* currentRawDataBlock = findWork(writerThreadData, configuration, currentBlockID);

		//We didn't find anything to do, wait for something to do.
		if(currentRawDataBlock == NULL)
		{
			std::cout << "WriterThread: Waiting for work to do..." << std::endl;

			//Wait for work to do
			writerThreadData->writeDataQueueConditionVariable.wait(workQueueLock, [&]
			{
				//Go through the work vector and see if there is anything to do
				for(uint32_t i = 0; i < writerThreadData->writeDataQueue.size(); ++i)
				{
					//Is this the next block that should be written AND all threads have finished processing it?
					if( currentBlockID == writerThreadData->writeDataQueue[i]->rawDataBlockID &&
							writerThreadData->writeDataQueue[i]->workerThreadsCompletedProcessing == configuration->numberOfWorkerThreads)
					{
						return true;
					}
				}

				return false;
			});


			std::cout << "WriterThread: Woken up!" << std::endl;

			//At this point we have the lock again
			//Get the current raw data block out of the workQueue
			currentRawDataBlock = findWork(writerThreadData, configuration, currentBlockID);
		}



		//We have a raw data block at this point, unlock the mutex
		workQueueLock.unlock();


		//Write out the data
		uint64_t bytesToWritePerFilterbank = currentRawDataBlock->usedDataLength / configuration->beamNum;

		std::cout << "WriterThread: Writing data..." << std::endl;

		for(uint32_t i = 0; i < writerThreadData->filterbankOutputVector.size(); ++i)
		{
			SigprocFilterbankOutput* currentFilterbank = writerThreadData->filterbankOutputVector[i];

			WriteSigprocOutputFile(currentFilterbank, currentRawDataBlock->packedRawData + (i* bytesToWritePerFilterbank),
					bytesToWritePerFilterbank);
		}


		//Do house keeping
		shouldShutdown = currentRawDataBlock->isLastBlock;
		currentBlockID += 1;


		//Pass this rawDataBlock back to the reader thread
		masterMailbox->writerReaderMailbox->Writer_PassRawDataBlockToReaderThread(currentRawDataBlock);


	}


	//Return and close the thread
}
