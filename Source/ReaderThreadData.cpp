/*
 * ReaderThreadData.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "../Header/ReaderThreadData.h"

ReaderThreadData::ReaderThreadData(std::vector<RawDataBlock*>* rawDataBlockQueue)
{

	this->rawDataBlockQueue = rawDataBlockQueue;
	this->rawDataBlockQueueMutex = new std::mutex();
	this->rawDataBlockQueueCV = new std::condition_variable();

}

ReaderThreadData::~ReaderThreadData()
{
	//Raw data blocks are freed elsewhere

	//Free the mutex and condition variable
	delete rawDataBlockQueueMutex;
	delete rawDataBlockQueueCV;
}
