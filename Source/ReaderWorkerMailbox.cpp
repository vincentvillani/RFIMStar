/*
 * ReaderWorkerMailbox.cpp
 *
 *  Created on: 7 May 2016
 *      Author: vincent
 */

#include "../Header/ReaderWorkerMailbox.h"

ReaderWorkerMailbox::ReaderWorkerMailbox(ReaderThreadData* readerThreadData, std::vector<WorkerThreadData*>* workerThreadDataVector)
{
	this->readerThreadData = readerThreadData;
	this->workerThreadDataVector = workerThreadDataVector;
}

ReaderWorkerMailbox::~ReaderWorkerMailbox()
{
	//Don't free anything, it is freed elsewhere
}

void ReaderWorkerMailbox::ReaderToWorkers_QueueRawDataBlock(RawDataBlock* rawDataBlock)
{

	//Take a raw data block and add it to each worker threads work queue
	for(uint32_t i = 0; i < workerThreadDataVector->size(); ++i)
	{

		{
			//Lock the the work queue mutex
			//When the lock guard goes out of scope the mutex will be unlocked
			std::lock_guard<std::mutex> workerQueueLockGuard( (*workerThreadDataVector)[i]->workQueueMutex );

			//Place the raw data block in the work queue
			(*workerThreadDataVector)[i]->workQueue.push(rawDataBlock);

		}

		//Let the worker thread know something was placed in their queue, so they can wake up if they are currently sleeping
		(*workerThreadDataVector)[i]->workQueueConditionVariable.notify_one();

	}

}


