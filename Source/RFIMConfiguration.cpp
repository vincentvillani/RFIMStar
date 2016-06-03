/*
 * RFIMConfiguration.cpp
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#include "../Header/RFIMConfiguration.h"



RFIMConfiguration::RFIMConfiguration()
{
	this->numberOfWorkerThreads = 0;
	this->windowSize = 0;
	this->channelNum = 0; //Set by the filterbank files
	this->beamNum = 0;
	this->dimensionsToReduce = 0;
	this->rawDataBlockNum = 0;
	this->numBitsPerSample = 0; //Set by the filterbank files

	this->inputFilenamePrefix = "";
	this->inputFilenamePostfix = "";

	this->outputFilenamePrefix = "";
	this->outputFilenamePostfix = "";

	this->generatingMask = false;

	powerThreshold = 0;
}


RFIMConfiguration::RFIMConfiguration(uint32_t workerThreadNum, uint32_t windowSize, uint32_t beamNum, uint32_t dimensionsToReduce, uint32_t rawDataBlockNum,
		std::string inputFilenamePrefix, std::string inputFilenamePostfix,
		std::string outputFilenamePrefix, std::string outputFilenamePostfix, bool generatingMask, RFIMMode mode)
{

	this->numberOfWorkerThreads = workerThreadNum;
	this->windowSize = windowSize;
	this->channelNum = 0; //Set by the filterbank files
	this->beamNum = beamNum;
	this->dimensionsToReduce = dimensionsToReduce;
	this->rawDataBlockNum = rawDataBlockNum;
	this->numBitsPerSample = 0; //Set by the filterbank files

	this->inputFilenamePrefix = inputFilenamePrefix;
	this->inputFilenamePostfix = inputFilenamePostfix;

	this->outputFilenamePrefix = outputFilenamePrefix;
	this->outputFilenamePostfix = outputFilenamePostfix;

	this->generatingMask = generatingMask;

	this->powerThreshold = 0;
	this->rfimMode = mode;
}



RFIMConfiguration::~RFIMConfiguration()
{
}



void RFIMConfiguration::setFilterbankVariables(SigprocFilterbank* filterbank)
{
	channelNum = (uint32_t)filterbank->get_nchans();
	numBitsPerSample = (uint32_t)filterbank->get_nbits();
}
