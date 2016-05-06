/*
 * RFIMConfiguration.h
 *
 *  Created on: 6 May 2016
 *      Author: vincentvillani
 */

#ifndef HEADER_RFIMCONFIGURATION_H_
#define HEADER_RFIMCONFIGURATION_H_

#include <stdint.h>

class RFIMConfiguration
{
public:
	RFIMConfiguration();
	RFIMConfiguration(uint32_t workerThreadNum, uint32_t windowSize, uint32_t channelSize, uint32_t batchSize, uint32_t beamNum,
			uint32_t rawDataBlockNum, uint32_t numBitsPerSample);
	~RFIMConfiguration();

	uint32_t numberOfWorkerThreads; //The number of worker threads to spawn
	uint32_t windowSize; //Number of samples to run RFIM on at a time
	uint32_t batchSize; //How many items of work a worker thread should compute for each iteration
	uint32_t channelNum; //Number of channels in the filterbank file
	uint32_t beamNum; //Number of beams
	uint32_t rawDataBlockNum; //Number of raw data blocks that can be used as I/O buffers
	uint32_t numBitsPerSample;
};


#endif /* HEADER_RFIMCONFIGURATION_H_ */
