/*
 * WorkerWriterMailbox.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */

#include "../Header/WorkerWriterMailbox.h"

#include <mutex>


WorkerWriterMailbox::WorkerWriterMailbox(WriterThreadData* writerThreadData, RFIMConfiguration* configuration)
{
	this->writerThreadData = writerThreadData;
	this->configuration = configuration;
}


WorkerWriterMailbox::~WorkerWriterMailbox()
{
}

	//Tell the writer thread that one thread is finished with this block
void WorkerWriterMailbox::Worker_NotifyFinishedWithBlock(RawDataBlock* rawDataBlock)
{
	//Get the writer threads workQueue lock
	std::lock_guard<std::mutex> writerQueueLock(writerThreadData->writeDataQueueMutex);

	bool isAlreadyInQueue = false;

	//Is this rawDataBlock already in the queue?
	for(uint64_t i = 0; i < writerThreadData->writeDataQueue.size(); ++i)
	{
		if(writerThreadData->writeDataQueue[i] == rawDataBlock)
		{
			isAlreadyInQueue = true;
			break;
		}
	}

	//If its not in the queue add it to the queue
	if(isAlreadyInQueue == false)
	{
		writerThreadData->writeDataQueue.push_back(rawDataBlock);
	}

	//Increment the rawDataBlocks completion value
	rawDataBlock->workerThreadsCompletedProcessing += 1;

	//Notify the writer thread that something is ready for processing if all worker threads have finished processing the current block
	if(rawDataBlock->workerThreadsCompletedProcessing == configuration->numberOfWorkerThreads)
	{
		writerThreadData->writeDataQueueConditionVariable.notify_one();
	}

	//Mutex is automatically unlocked when the lock_guard is destroyed

}
