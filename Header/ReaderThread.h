/*
 * ReaderThread.h
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_READERTHREAD_H_
#define HEADER_READERTHREAD_H_

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "../DeadSigproc/DeadSigproc.h"

#include "ReaderThreadData.h"
#include "RFIMConfiguration.h"

//The reader threads 'main' function
void ReaderThreadMain(std::vector<SigprocFilterbank*>* filterbankVector, ReaderThreadData* RTD, RFIMConfiguration* RFIMConfig);

#endif /* HEADER_READERTHREAD_H_ */
