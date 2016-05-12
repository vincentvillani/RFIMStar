/*
 * WriterReaderMailbox.h
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_WRITERREADERMAILBOX_H_
#define HEADER_WRITERREADERMAILBOX_H_

#include "ReaderThreadData.h"
#include "../DeadSigproc/RawDataBlock.h"

class WriterReaderMailbox
{

public:

	ReaderThreadData* readerThreadData;

	WriterReaderMailbox(ReaderThreadData* readerThreadData);
	~WriterReaderMailbox();

	void Writer_PassRawDataBlockToReaderThread(RawDataBlock* rawDataBlock);

};


#endif /* HEADER_WRITERREADERMAILBOX_H_ */
