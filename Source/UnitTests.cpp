/*
 * UnitTests.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */


#include "../Header/UnitTests.h"

#include <sstream>
#include <stdio.h>


void RunAllUniTests()
{
	PackingUnpackingUnitTest();

	std::cout << "All unit tests complete!" << std::endl;
}


//Pack and unpack filterbank files without doing RFIM
//Tests to see if the unpack and pack functionality works correctly
//The input and output filterbank files should be identical
void PackingUnpackingUnitTest()
{
	uint32_t beamNum = 13;
	uint32_t rawDataBlockNum = 10;
	uint32_t numberOfWorkerThreads = 10;
	uint32_t windowSize = 15625;

	std::string inputFilenamePrefix = "/lustre/projects/p002_swin/surveys/SUPERB/2016-01-05-12:07:06/";
	std::string inputFilenamePostfix = "/2016-01-05-12:07:06.fil";

	std::string outputFilenamePrefix = "/lustre/projects/p002_swin/vvillani/";
	std::string outputFilenamePostfix = ".fil";

	RFIMConfiguration configuration = RFIMConfiguration(numberOfWorkerThreads, windowSize, beamNum, rawDataBlockNum,
			inputFilenamePrefix, inputFilenamePostfix,
			outputFilenamePrefix, outputFilenamePostfix);


	//Run the routine
	RFIMStarRoutine(&configuration);

	std::cout << "RFIM STAR ROUTINE FINISHED!!!!" << std::endl;

	std::stringstream ssInput;
	std::stringstream ssOutput;



	//After finishing, open up each filterbank file and make sure that they are byte for byte the same!
	for(uint32_t i = 1; i < configuration.beamNum + 1; ++i)
	{

		ssInput << inputFilenamePrefix;
		ssOutput << outputFilenamePrefix;

		if(i < 10)
		{
			ssInput << "0" << i;
			ssOutput << "0" << i;
		}
		else
		{
			ssInput << i;
			ssOutput << i;
		}

		ssInput << inputFilenamePostfix;
		ssOutput << outputFilenamePostfix;

		SigprocFilterbank* originalFilterbank = new SigprocFilterbank(ssInput.str());
		SigprocFilterbank* outputFilterbank = new SigprocFilterbank(ssOutput.str());

		//Reset the string streams
		ssInput.str("");
		ssOutput.str("");

		//Check that both filterbanks are the same size
		if(originalFilterbank->packedDataByteSize != outputFilterbank->packedDataByteSize)
		{
			fprintf(stderr, "PackingUnpackingUnitTest: filterbanks are not the same size!\n");
			exit(1);
		}

		//Read both filterbanks to a buffer
		unsigned char* originalFilterbankData = new unsigned char[originalFilterbank->packedDataByteSize];
		unsigned char* outputFilterbankData = new unsigned char[outputFilterbank->packedDataByteSize];

		uint64_t originalFilterbankBytesRead = -1;
		uint64_t outputFilterbankBytesRead = -2;

		ReadFilterbankData(originalFilterbank, originalFilterbankData, originalFilterbank->packedDataByteSize,
				&originalFilterbankBytesRead);
		ReadFilterbankData(outputFilterbank, outputFilterbankData, outputFilterbank->packedDataByteSize,
						&outputFilterbankBytesRead);

		//Did we read the same amount of bytes from each?
		if(originalFilterbankBytesRead != outputFilterbankBytesRead)
		{
			fprintf(stderr, "PackingUnpackingUnitTest: Bytes read from each filterbank is not the same size!\n");
			exit(1);
		}


		//Go through byte by byte and check that each byte is the same as the original
		for(uint64_t currentByteIndex = 0; currentByteIndex < originalFilterbank->packedDataByteSize; ++currentByteIndex)
		{
			if(originalFilterbankData[currentByteIndex] != outputFilterbankData[currentByteIndex])
			{
				unsigned char originalByte = originalFilterbankData[currentByteIndex];
				unsigned char outputByte = outputFilterbankData[currentByteIndex];

				fprintf(stderr, "filterbank files: %u\n", i);
				fprintf(stderr, "PackingUnpackingUnitTest: Bytes do not have the same value!\n");
				fprintf(stderr, "Total byte Size: %zu\ncurrentByteIndex: %llu\n", originalFilterbank->packedDataByteSize, currentByteIndex);
				fprintf(stderr, "originalByte: %u\noutputByte: %u\n", originalByte, outputByte);
				//exit(1);
			}
		}


		//Free all allocated data
		delete originalFilterbank;
		delete outputFilterbank;

		delete [] originalFilterbankData;
		delete [] outputFilterbankData;

	}




}
