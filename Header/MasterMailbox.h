/*
 * MasterMailbox.h
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_MASTERMAILBOX_H_
#define HEADER_MASTERMAILBOX_H_

#include "ReaderWorkerMailbox.h"
#include "WorkerWriterMailbox.h"
#include "WriterReaderMailbox.h"

class MasterMailbox
{
public:
	ReaderWorkerMailbox* readerWorkerMailbox;
	WorkerWriterMailbox* workerWriterMailbox;
	WriterReaderMailbox* writerReaderMailbox;

	MasterMailbox(ReaderWorkerMailbox* RWM, WorkerWriterMailbox* workerWriterMailbox, WriterReaderMailbox* writerReaderMailbox);
	~MasterMailbox();
};

#endif /* HEADER_MASTERMAILBOX_H_ */
