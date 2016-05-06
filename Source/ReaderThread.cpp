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
