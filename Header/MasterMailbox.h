/*
 * MasterMailbox.h
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_MASTERMAILBOX_H_
#define HEADER_MASTERMAILBOX_H_

#include "ReaderWorkerMailbox.h"

class MasterMailbox
{
public:
	ReaderWorkerMailbox* readerWorkerMailbox;

	MasterMailbox(ReaderWorkerMailbox* RWM);
	~MasterMailbox();
};

#endif /* HEADER_MASTERMAILBOX_H_ */
