/*
 * WorkerWriterMailbox.h
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_WORKERWRITERMAILBOX_H_
#define HEADER_WORKERWRITERMAILBOX_H_


#include "WorkerThreadData.h"
#include "WriterThreadData.h"
#include "RFIMConfiguration.h"
#include "../DeadSigproc/RawDataBlock.h"

class WorkerWriterMailbox
{
public:

	WriterThreadData* writerThreadData;
	RFIMConfiguration* configuration;

	WorkerWriterMailbox(WriterThreadData* writerThreadData, RFIMConfiguration* configuration);
	~WorkerWriterMailbox();

	//Tell the writer thread that one thread is finished with this block
	void Worker_NotifyFinishedWithBlock(RawDataBlock* rawDataBlock);
};



#endif /* HEADER_WORKERWRITERMAILBOX_H_ */
