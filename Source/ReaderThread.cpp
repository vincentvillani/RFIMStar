/*
 * ReaderThread.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include <stdint.h>

#include "../Header/ReaderThread.h"
#include "../DeadSigproc/DeadSigproc.h"


//Filterbanks are already open and ready to go
void ReaderThreadMean(std::vector<SigprocFilterbank*> filterbankVector, ReaderThreadData* RTD)
{

	/*
	//1. Open all the input filterbank files
	for(uint32_t i = 0; i < filterbankFilenamesVector.size(); ++i)
	{
		SigprocFilterbank* filterbankFile = new SigprocFilterbank(filterbankFilenamesVector[i]);

		//Add it to the filterbank vector
		filterbankVector.push_back(filterbankFile);
	}
	*/

	//2. Setup the raw data blocks based on the configuration (DONE SOMEWHERE ELSE)

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


}
