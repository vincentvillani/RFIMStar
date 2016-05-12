/*
 * RFIMStar.h
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_RFIMSTAR_H_
#define HEADER_RFIMSTAR_H_

#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <functional>
#include <algorithm>


#include "../Header/RFIMConfiguration.h"

#include "../Header/ReaderThread.h"
#include "../Header/WorkerThread.h"
#include "../Header/WriterThread.h"


void RFIMStarRoutine(RFIMConfiguration* configuration);


#endif /* HEADER_RFIMSTAR_H_ */
