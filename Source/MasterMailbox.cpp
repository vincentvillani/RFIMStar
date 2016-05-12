/*
 * MasterMailbox.cpp
 *
 *  Created on: 9 May 2016
 *      Author: vincentvillani
 */

#include "../Header/MasterMailbox.h"


MasterMailbox::MasterMailbox(ReaderWorkerMailbox* RWM, WorkerWriterMailbox* workerWriterMailbox, WriterReaderMailbox* writerReaderMailbox)
{
	this->readerWorkerMailbox = RWM;
	this->workerWriterMailbox = workerWriterMailbox;
	this->writerReaderMailbox = writerReaderMailbox;
}


MasterMailbox::~MasterMailbox()
{
}
