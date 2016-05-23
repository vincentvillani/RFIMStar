/*
 * WorkerThreadData.h
 *
 *  Created on: 7 May 2016
 *      Author: vincent
 */

#ifndef HEADER_WORKERTHREADDATA_H_
#define HEADER_WORKERTHREADDATA_H_

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <stdint.h>

#include "../DeadSigproc/RawDataBlock.h"

class WorkerThreadData
{

public:

	std::queue<RawDataBlock*> workQueue;
	std::mutex workQueueMutex;
	std::condition_variable workQueueConditionVariable;

	uint64_t numberOfDimensionsRemoved;

	WorkerThreadData();
	~WorkerThreadData();

};



#endif /* HEADER_WORKERTHREADDATA_H_ */
