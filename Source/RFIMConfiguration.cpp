/*
 * RFIMConfiguration.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "../Header/RFIMConfiguration.h"

RFIMConfiguration::RFIMConfiguration()
{

	numberOfWorkerThreads = 0;
	windowSize = 0;
	batchSize = 0;
	channelNum = 0;
	beamNum = 0;
	rawDataBlockNum = 0;
	numBitsPerSample = 0;

}


RFIMConfiguration::RFIMConfiguration(uint32_t workerThreadNum, uint32_t windowSize, uint32_t channelSize, uint32_t batchSize,
		uint32_t beamNum, uint32_t rawDataBlockNum, uint32_t numBitsPerSample)
{

	this->numberOfWorkerThreads = workerThreadNum;
	this->windowSize = windowSize;
	this->channelNum = channelSize;
	this->batchSize = batchSize;
	this->beamNum = beamNum;
	this->rawDataBlockNum = rawDataBlockNum;
	this->numBitsPerSample = numBitsPerSample;

}



RFIMConfiguration::~RFIMConfiguration()
{
}
