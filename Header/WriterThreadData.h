/*
 * WriterThreadData.h
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_WRITERTHREADDATA_H_
#define HEADER_WRITERTHREADDATA_H_

#include <vector>
#include <mutex>
#include <condition_variable>

#include "../DeadSigproc/DeadSigproc.h"

class WriterThreadData
{

public:

	std::vector<RawDataBlock*> writeDataQueue;
	std::mutex writeDataQueueMutex;
	std::condition_variable writeDataQueueConditionVariable;
	std::vector<SigprocFilterbankOutput*> filterbankOutputVector;
	SigprocFilterbankOutput* maskFilterbank;

	WriterThreadData(std::vector<SigprocFilterbankOutput*> filterbankOutputVector, SigprocFilterbankOutput* maskFilterbank);
	~WriterThreadData();

};


#endif /* HEADER_WRITERTHREADDATA_H_ */
