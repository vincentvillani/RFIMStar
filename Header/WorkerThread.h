/*
 * WorkerThread.h
 *
 *  Created on: 9 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_WORKERTHREAD_H_
#define HEADER_WORKERTHREAD_H_

#include "MasterMailbox.h"

#include <stdint.h>

#include "RFIMConfiguration.h"
#include "RFIMMemoryBlock.h"


void WorkerThreadMain(uint32_t workerThreadID, WorkerThreadData* threadData, MasterMailbox* masterMailbox, RFIMConfiguration* configuration);


void WorkerThreadUnpackData(uint32_t workerThreadID, RawDataBlock* rawDataBlock, RFIMMemoryBlock* rfimMemoryBlock,
		RFIMConfiguration* configuration);


void WorkerThreadMultiplexData(RFIMMemoryBlock* rfimMemoryBlock, RFIMConfiguration* configuration);

void WorkerThreadDeMultiplexData(RFIMMemoryBlock* rfimMemoryBlock, RFIMConfiguration* configuration);

void WorkerThreadPackData(uint32_t workerThreadID, RawDataBlock* rawDataBlock, RFIMMemoryBlock* rfimMemoryBlock,
		RFIMConfiguration* configuration);



#endif /* HEADER_WORKERTHREAD_H_ */
