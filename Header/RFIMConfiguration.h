/*
 * RFIMConfiguration.h
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_RFIMCONFIGURATION_H_
#define HEADER_RFIMCONFIGURATION_H_

#include <stdint.h>
#include <string>

#include "../DeadSigproc/SigprocFilterbank.h"

enum RFIMMode
{
	ZERO,
	ATTENUATE
};

class RFIMConfiguration
{
public:

	uint32_t numberOfWorkerThreads; //The number of worker threads to spawn
	uint32_t windowSize; //Number of samples to run RFIM on at a time
	uint32_t channelNum; //Number of channels in the filterbank file, set by the filterbank files
	uint32_t beamNum; //Number of beams
	uint32_t dimensionsToReduce; //Number of dimensions to reduce
	uint32_t rawDataBlockNum; //Number of raw data blocks that can be used as I/O buffers
	uint32_t numBitsPerSample; //Number of bits per sample, set by the filterbank files
	//uint64_t totalSamplesToProcess; //Number of samples to process if you don't want to process the whole file
	bool generatingMask; //Do we want to generate a mask?
	float powerThreshold;
	RFIMMode rfimMode;


	std::string inputFilenamePrefix;
	std::string inputFilenamePostfix;

	std::string outputFilenamePrefix;
	std::string outputFilenamePostfix;

	RFIMConfiguration();

	RFIMConfiguration(uint32_t workerThreadNum, uint32_t windowSize, uint32_t beamNum, uint32_t dimensionsToReduce,
			uint32_t rawDataBlockNum,
			std::string inputFilenamePrefix, std::string inputFilenamePostfix,
			std::string outputFilenamePrefix, std::string outputFilenamePostfix, bool generatingMask, RFIMMode mode);

	~RFIMConfiguration();


	//Set the variables that need to be set from a filterbank
	void setFilterbankVariables(SigprocFilterbank* filterbank);


};


#endif /* HEADER_RFIMCONFIGURATION_H_ */
