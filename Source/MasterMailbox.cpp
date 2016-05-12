/*
 * MasterMailbox.cpp
 *
 *  Created on: 9 May 2016
 *      Author: vincentvillani
 */

#include "../Header/MasterMailbox.h"


MasterMailbox::MasterMailbox(ReaderWorkerMailbox* RWM, WorkerWriterMailbox* workerWriterMailbox)
{
	this->readerWorkerMailbox = RWM;
	this->workerWriterMailbox = workerWriterMailbox;
}


MasterMailbox::~MasterMailbox()
{
}
