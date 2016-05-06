/*
 * ReaderThreadData.h
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#ifndef READERTHREADDATA_H_
#define READERTHREADDATA_H_

#include <vector>
#include <mutex>
#include <condition_variable>

#include "../DeadSigproc/DeadSigproc.h"

class ReaderThreadData
{
public:
	ReaderThreadData(std::vector<RawDataBlock*>* rawDataBlockQueue);
	~ReaderThreadData();

	std::vector<RawDataBlock*>* rawDataBlockQueue;
	std::mutex* rawDataBlockQueueMutex;
	std::condition_variable* rawDataBlockQueueCV;
};


#endif /* READERTHREADDATA_H_ */
