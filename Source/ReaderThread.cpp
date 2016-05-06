/*
 * ReaderThread.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "../Header/ReaderThread.h"

#include <stdint.h>
#include <vector>




//Filterbanks are already open and ready to go
void ReaderThreadMain(std::vector<SigprocFilterbank*> filterbankVector, ReaderThreadData* RTD)
{

	//TODO: If one of the filterbank files reaches the EOF, we cut the rest of them off at that point and pretend thats the last sample
	//for all the other filterbank files
	while(filterbankVector[0]->hasReachedEOF() == false)
	{

		//Aquire the lock
		std::unique_lock<std::mutex> readerThreadLockGuard(*RTD->rawDataBlockQueueMutex);

		//Do we have to wait or can we start work right away?
		if(RTD->rawDataBlockQueue->size() == 0)
		{
			//We have to wait for a buffer to use

			//This checks the predicate, if it is false it releases the lock and puts the thread to sleep
			//Once someone has alerted the condition variable the lock is aquired again and the predicate is checked again
			//If it is true it will keep the lock aquired
			RTD->rawDataBlockQueueCV->wait(readerThreadLockGuard, [&]{return RTD->rawDataBlockQueue->size();});
		}


		//Once we wake and the predicate is satifised we should have the lock again.

		//Get a reference to it and then remove it from the queue
		RawDataBlock* currentBuffer =  *(RTD->rawDataBlockQueue->end() - 1);
		RTD->rawDataBlockQueue->erase( RTD->rawDataBlockQueue->end() - 1);


		//At this point we should have a buffer, so release the mutex
		readerThreadLockGuard.release();

		//TODO: Read in filterbank data

		//Send it off to the worker threads when done

	}


	//3. Start reading in data (Raw data blocks should be setup at this point)


	//4. Remove the data blocks and place them in the worker threads queue

	//5. Go back to step 3 if there are spare data blocks & more data to read in.
	//If not go to sleep and wait till there are

	//6. Free all data blocks and filterbanks

	/*
	//Free fiterbanks
	for(uint32_t i = 0; i < filterbankVector.size(); ++i)
	{
		delete filterbankVector[i];
	}
	*/

	//7. return

	std::cout << "Reader thread finished" << std::endl;

}
