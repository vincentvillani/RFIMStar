/*
 * WriterThread.h
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_WRITERTHREAD_H_
#define HEADER_WRITERTHREAD_H_

#include "WriterThread.h"
#include "MasterMailbox.h"
#include "RFIMConfiguration.h"


void WriterThreadMain(WriterThreadData* writerThreadData, RFIMConfiguration* configuration, MasterMailbox* masterMailbox);



#endif /* HEADER_WRITERTHREAD_H_ */
