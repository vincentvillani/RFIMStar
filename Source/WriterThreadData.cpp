/*
 * WriterThreadData.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */


#include "../Header/WriterThreadData.h"


WriterThreadData::WriterThreadData(std::vector<SigprocFilterbankOutput*> filterbankOutputVector)
{
	this->filterbankOutputVector = filterbankOutputVector;
}


WriterThreadData::~WriterThreadData()
{

}
