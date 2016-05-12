/*
 * WriterReaderMailbox.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */


#include "../Header/WriterReaderMailbox.h"


#include <mutex>


WriterReaderMailbox::WriterReaderMailbox(ReaderThreadData* readerThreadData)
{
	this->readerThreadData = readerThreadData;
}

WriterReaderMailbox::~WriterReaderMailbox()
{

}

void WriterReaderMailbox::Writer_PassRawDataBlockToReaderThread(RawDataBlock* rawDataBlock)
{

	//Reset worker threads completed processing data
	rawDataBlock->workerThreadsCompletedProcessing = 0;


	//Get the mutex before adding anything to the queue
	std::lock_guard<std::mutex> readerThreadWorkQueueLock(*(readerThreadData->rawDataBlockQueueMutex));

	//Add it to the reader threads work queue
	readerThreadData->rawDataBlockQueue->push_back(rawDataBlock);

	//Notify the reader thread if it's sleeping
	readerThreadData->rawDataBlockQueueCV->notify_one();


}
