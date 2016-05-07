/*
 * ReaderWorkerMailbox.h
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_READERWORKERMAILBOX_H_
#define HEADER_READERWORKERMAILBOX_H_



#include "ReaderThreadData.h"
#include "WorkerThreadData.h"

#include "../DeadSigproc/RawDataBlock.h"

class ReaderWorkerMailbox
{
public:

	ReaderThreadData* readerThreadData;
	std::vector<WorkerThreadData*>* workerThreadDataVector;

	ReaderWorkerMailbox(ReaderThreadData* readerThreadData, std::vector<WorkerThreadData*>* workerThreadDataVector);
	~ReaderWorkerMailbox();

	void ReaderToWorkers_QueueRawDataBlock(RawDataBlock* rawDataBlock);
};


#endif /* HEADER_READERWORKERMAILBOX_H_ */
