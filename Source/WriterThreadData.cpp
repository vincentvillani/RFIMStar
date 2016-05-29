/*
 * WriterThreadData.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */


#include "../Header/WriterThreadData.h"


WriterThreadData::WriterThreadData(std::vector<SigprocFilterbankOutput*> filterbankOutputVector,
		SigprocFilterbankOutput* maskOutputFilterbank)
{
	this->filterbankOutputVector = filterbankOutputVector;
	this->maskFilterbank = maskOutputFilterbank;
}


WriterThreadData::~WriterThreadData()
{

}
