/*
 * UnitTests.cpp
 *
 *  Created on: 12 May 2016
 *      Author: vincentvillani
 */


#include "../Header/UnitTests.h"

#include <sstream>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "../Header/RFIMHelperFunctions.h"
#include "../Header/RFIMMemoryBlock.h"
#include "../Header/WorkerThread.h"

void RunAllUnitTests()
{


	//PCAUnitTest();
	//DetailedUnitTest();

	//MultiplexDemultiplexUnitTest();

	//MeanUnitTest();
	CovarianceMatrixUnitTest();
	//EigenvectorSolverUnitTest();
	//ProjectionDeprojectionUnitTest();
	//Remove1DProjectionDeprojectionUnitTest();

	//PackingUnpackingUnitTest();

	std::cout << "All unit tests complete!" << std::endl;
}


void PCAUnitTest()
{
	float data[20];
	data[0] = 0.69f;
	data[1] = 0.49f;
	data[2] = -1.31f;
	data[3] = -1.21f;
	data[4] = 0.39f;
	data[5] = 0.99f;
	data[6] = 0.09f;
	data[7] = 0.29f;
	data[8] = 1.29f;
	data[9] = 1.09f;
	data[10] = 0.49f;
	data[11] = 0.79f;
	data[12] = 0.19f;
	data[13] = -0.31f;
	data[14] = -0.81f;
	data[15] = -0.81f;
	data[16] = -0.31f;
	data[17] = -0.31f;
	data[18] = -0.71f;
	data[19] = -1.01f;


	float eigenvectorDecendingSignificance[4];
	eigenvectorDecendingSignificance[0] = -0.667873399f;
	eigenvectorDecendingSignificance[1] = -0.735178656f;
	eigenvectorDecendingSignificance[2] = -0.735178656f;
	eigenvectorDecendingSignificance[3] = 0.667873399f;


	/*
	float eigenvectorAcendingSignificance[4];
	eigenvectorAcendingSignificance[0] = -0.735178656f;
	eigenvectorAcendingSignificance[1] = 0.667873399f;
	eigenvectorAcendingSignificance[2] = -0.667873399f;
	eigenvectorAcendingSignificance[3] = -0.735178656f;
	*/


	uint32_t valuesPerSample = 2;
	uint32_t numberOfSamples = 10;
	//uint32_t


	RFIMMemoryBlock rfimMem = RFIMMemoryBlock(valuesPerSample, numberOfSamples, 1, 1);
	memcpy(rfimMem.h_covarianceMatrix, &eigenvectorDecendingSignificance, sizeof(float) * 4);
	memcpy(rfimMem.h_inputSignal, &data, sizeof(float) * 20);

	//Print out the deprojected results inside this method
	EigenReductionAndFiltering(&rfimMem);


	for(uint32_t i = 0; i < 20; ++i)
	{
		printf("PCAUnitTest: Deprojected[%lu]: %f\n", i,  rfimMem.h_inputSignal[i]);
	}

}



void DetailedUnitTest()
{

	uint32_t valuesPerSample = 2;
	uint32_t sampleNum = 2;
	uint32_t channelNum = 2;
	uint32_t dimensionToReduce = 1;

	uint32_t numberOfThreads = 2;

	uint32_t nBits = 2;
	uint32_t rawDataArrayLength = 4;
	uint32_t filterbankByteSize = rawDataArrayLength / 2;



	//OLD DATA
	//[(0 3 2 1), (1 2 2 3), (0 2 1 1), (1 1 3 1)], [(3 3 1 1), (0 2 2 1), (3 1 0 2), (0 2 0 2)]

	//Packed filterbank data
	//[(0 3 2 1), (1 2 2 3)], [(3 3 1 1), (0 2 2 1)]

	unsigned char rawPackedFilterbankOne[2];
	rawPackedFilterbankOne[0] = 57;
	rawPackedFilterbankOne[1] = 107;
	//rawPackedFilterbankOne[2] = 37;
	//rawPackedFilterbankOne[3] = 93;

	unsigned char rawPackedFilterbankTwo[2];
	rawPackedFilterbankTwo[0] = 245;
	rawPackedFilterbankTwo[1] = 41;
	//rawPackedFilterbankTwo[2] = 210;
	//rawPackedFilterbankTwo[3] = 51;




	RFIMConfiguration configuration = RFIMConfiguration(numberOfThreads, sampleNum, valuesPerSample,
			dimensionToReduce, 1, "", "", "", "");
	configuration.channelNum = channelNum; //USUALLY SET BY READING FILTERBANK FILES
	configuration.numBitsPerSample = nBits; //USUALLY SET BY READING FILTERBANK FILES



	//Copy the filterbank data across to the raw data block
	RawDataBlock rawDataBlock = RawDataBlock(0, rawDataArrayLength, nBits);
	memcpy(rawDataBlock.packedRawData, rawPackedFilterbankOne, filterbankByteSize);
	memcpy(rawDataBlock.packedRawData + filterbankByteSize, rawPackedFilterbankTwo, filterbankByteSize);
	rawDataBlock.usedDataLength = rawDataArrayLength;




	RFIMMemoryBlock rfimMemoryBlockOne = RFIMMemoryBlock(valuesPerSample, sampleNum, dimensionToReduce, channelNum);
	RFIMMemoryBlock rfimMemoryBlockTwo = RFIMMemoryBlock(valuesPerSample, sampleNum, dimensionToReduce, channelNum);


	//Unpack the data
	WorkerThreadUnpackData(0, &rawDataBlock, &rfimMemoryBlockOne, &configuration);
	WorkerThreadUnpackData(1, &rawDataBlock, &rfimMemoryBlockTwo, &configuration);



	//Print out the unpacked data for block one
	//Should be (0 3 2 1), (3 3 1 1)
	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("UnpackedBlockOne[%llu]: %f\n", i, rfimMemoryBlockOne.h_outputSignal[i]);
	}

	printf("\n");

	//Print out the unpacked data for block two
	//Should be (1 2 2 3), (0 2 2 1)
	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("UnpackedBlockTwo[%llu]: %f\n", i, rfimMemoryBlockTwo.h_outputSignal[i]);
	}


	printf("\n");

	//Multiplex the data
	WorkerThreadMultiplexData(&rfimMemoryBlockOne, &configuration);
	WorkerThreadMultiplexData(&rfimMemoryBlockTwo, &configuration);



	//Print out the multiplexed data

	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("MultiplexedBlockOne[%llu]: %f\n", i, rfimMemoryBlockOne.h_inputSignal[i]);
	}

	printf("\n");

	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("MultiplexedBlockTwo[%llu]: %f\n", i, rfimMemoryBlockTwo.h_inputSignal[i]);
	}


	//RFIM
	//Calculate the window size (AKA number of samples in RFIMStar jargon) of this iteration,
	//most of the time it will always be the same, unless we are at the end of the
	//filterbank files
	//This is so when we do RFIM we ignore samples that don't exist as we reach near the end of a file
	rfimMemoryBlockOne.h_numberOfSamples = (8 * (rawDataBlock.usedDataLength / configuration.beamNum)) /
			(configuration.channelNum * configuration.numberOfWorkerThreads * configuration.numBitsPerSample);
	rfimMemoryBlockTwo.h_numberOfSamples = (8 * (rawDataBlock.usedDataLength / configuration.beamNum)) /
			(configuration.channelNum * configuration.numberOfWorkerThreads * configuration.numBitsPerSample);






	//Calculate the covariance matrices
	CalculateCovarianceMatrix(&rfimMemoryBlockOne);
	CalculateCovarianceMatrix(&rfimMemoryBlockTwo);

	printf("\n");

	//print out the covariance matrices
	for(uint32_t i = 0; i < channelNum; ++i)
	{
		uint32_t covOffset = i * valuesPerSample * valuesPerSample;

		for(uint32_t j = 0; j < valuesPerSample * valuesPerSample; ++j)
		{
			printf("CovMatBlockOne[%llu][%llu]: %f\n", i, j, rfimMemoryBlockOne.h_covarianceMatrix[covOffset + j]);
		}

	}

	printf("\n");

	for(uint32_t i = 0; i < channelNum; ++i)
	{
		uint32_t covOffset = i * valuesPerSample * valuesPerSample;

		for(uint32_t j = 0; j < valuesPerSample * valuesPerSample; ++j)
		{
			printf("CovMatBlockTwo[%llu][%llu]: %f\n", i, j, rfimMemoryBlockTwo.h_covarianceMatrix[covOffset + j]);
		}

	}


	printf("\n");





	//Solve for eigenvalues
	EigenvalueSolver(&rfimMemoryBlockOne);
	EigenvalueSolver(&rfimMemoryBlockTwo);



	//print out the eigenvectors
	for(uint32_t i = 0; i < channelNum; ++i)
	{
		uint32_t covOffset = i * valuesPerSample * valuesPerSample;

		for(uint32_t j = 0; j < valuesPerSample * valuesPerSample; ++j)
		{
			printf("EigenMatBlockOne[%llu][%llu]: %f\n", i, j, rfimMemoryBlockOne.h_covarianceMatrix[covOffset + j]);
		}

	}

	printf("\n");

	for(uint32_t i = 0; i < channelNum; ++i)
	{
		uint32_t covOffset = i * valuesPerSample * valuesPerSample;

		for(uint32_t j = 0; j < valuesPerSample * valuesPerSample; ++j)
		{
			printf("EigenMatBlockTwo[%llu][%llu]: %f\n", i, j, rfimMemoryBlockTwo.h_covarianceMatrix[covOffset + j]);
		}

	}


	printf("\n");





	//Do projection and deprojection
	EigenReductionAndFiltering(&rfimMemoryBlockOne);
	EigenReductionAndFiltering(&rfimMemoryBlockTwo);


	//Print out the signal after
	for(uint32_t j = 0; j < valuesPerSample * sampleNum * channelNum; ++j)
	{
		printf("FilteredMatBlockOne[%llu]: %f\n", j, rfimMemoryBlockOne.h_inputSignal[j]);
	}

	printf("\n");

	for(uint32_t j = 0; j < valuesPerSample * sampleNum * channelNum; ++j)
	{
		printf("FilteredMatBlockTwo[%llu]: %f\n", j, rfimMemoryBlockTwo.h_inputSignal[j]);
	}

	printf("\n");



	//DeMmutliplex the signal
	WorkerThreadDeMultiplexData(&rfimMemoryBlockOne, &configuration);
	WorkerThreadDeMultiplexData(&rfimMemoryBlockTwo, &configuration);

	//Print out the signals

	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("DemultiplexBlockOne[%llu]: %f\n", i, rfimMemoryBlockOne.h_outputSignal[i]);
	}

	printf("\n");

	//Print out the unpacked data for block two
	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("DemultiplexBlockTwo[%llu]: %f\n", i, rfimMemoryBlockTwo.h_outputSignal[i]);
	}


	printf("\n");

	//Pack the data back into the raw data block
	WorkerThreadPackData(0, &rawDataBlock, &rfimMemoryBlockOne, &configuration);
	WorkerThreadPackData(1, &rawDataBlock, &rfimMemoryBlockTwo, &configuration);

	//Print packed data chars
	for(uint32_t i = 0; i < rawDataArrayLength; ++i)
	{
		printf("packedDataChars[%lu]: %u\n", i, rawDataBlock.packedRawData[i]);
	}

	printf("\n");

	//Copy the packed data into the temp block, unpack it again and print it out
	RawDataBlock tempRawDataBlock = RawDataBlock(0, rawDataArrayLength, nBits);
	tempRawDataBlock.usedDataLength = rawDataArrayLength;
	RFIMMemoryBlock tempRFIMBlock1 = RFIMMemoryBlock(valuesPerSample, sampleNum, dimensionToReduce, channelNum);
	RFIMMemoryBlock tempRFIMBlock2 = RFIMMemoryBlock(valuesPerSample, sampleNum, dimensionToReduce, channelNum);

	memcpy(tempRawDataBlock.packedRawData, rawDataBlock.packedRawData, rawDataArrayLength);

	WorkerThreadUnpackData(0, &tempRawDataBlock, &tempRFIMBlock1, &configuration);
	WorkerThreadUnpackData(1, &tempRawDataBlock, &tempRFIMBlock2, &configuration);

	//Print the packed->unpacked data
	//Print out the unpacked data for block one
	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("OutputBlockOne[%llu]: %f\n", i, tempRFIMBlock1.h_outputSignal[i]);
	}

	printf("\n");

	//Print out the unpacked data for block two
	for(uint32_t i = 0; i < sampleNum * channelNum * valuesPerSample; ++i)
	{
		printf("OutputBlockTwo[%llu]: %f\n", i, tempRFIMBlock2.h_outputSignal[i]);
	}


	//RFIM(&rfimMemoryBlockOne);
	//RFIM(&rfimMemoryBlockTwo);


}




void MultiplexDemultiplexUnitTest()
{
	uint32_t numberOfBeams = 2;
	uint32_t freqChannelNum = 3;
	uint32_t numberOfSamples = 4;
	uint32_t dimensionsToReduce = 0;
	uint32_t numberOfWorkerThreads = 1;
	uint32_t numberOfRawDatablocks = 0;

	//Setup memory
	RFIMMemoryBlock* RFIMMemBlock = new RFIMMemoryBlock(numberOfBeams, numberOfSamples, dimensionsToReduce, freqChannelNum);
	RFIMConfiguration* configuration = new RFIMConfiguration(numberOfWorkerThreads, numberOfSamples, numberOfBeams, dimensionsToReduce,
			numberOfRawDatablocks, "", "", "", "");
	configuration->channelNum = freqChannelNum; //Usually set by the filterbank files, so we have to set it manually here

	//Setup the signal
	uint32_t totalSignalLength = numberOfBeams * numberOfSamples * freqChannelNum;

	for(uint32_t i = 0; i < totalSignalLength; ++i)
	{
		RFIMMemBlock->h_outputSignal[i] = i;
	}

	//Multiplex the data
	WorkerThreadMultiplexData(RFIMMemBlock, configuration);


	float expectedResultsMultipexResults[numberOfBeams * freqChannelNum * numberOfSamples];

	expectedResultsMultipexResults[0] = 0;
	expectedResultsMultipexResults[1] = 12;
	expectedResultsMultipexResults[2] = 3;
	expectedResultsMultipexResults[3] = 15;
	expectedResultsMultipexResults[4] = 6;
	expectedResultsMultipexResults[5] = 18;
	expectedResultsMultipexResults[6] = 9;
	expectedResultsMultipexResults[7] = 21;
	expectedResultsMultipexResults[8] = 1;
	expectedResultsMultipexResults[9] = 13;
	expectedResultsMultipexResults[10] = 4;
	expectedResultsMultipexResults[11] = 16;
	expectedResultsMultipexResults[12] = 7;
	expectedResultsMultipexResults[13] = 19;
	expectedResultsMultipexResults[14] = 10;
	expectedResultsMultipexResults[15] = 22;
	expectedResultsMultipexResults[16] = 2;
	expectedResultsMultipexResults[17] = 14;
	expectedResultsMultipexResults[18] = 5;
	expectedResultsMultipexResults[19] = 17;
	expectedResultsMultipexResults[20] = 8;
	expectedResultsMultipexResults[21] = 20;
	expectedResultsMultipexResults[22] = 11;
	expectedResultsMultipexResults[23] = 23;




	//Check the results of multiplexing
	for(uint32_t i = 0; i < totalSignalLength; ++i)
	{
		if(RFIMMemBlock->h_inputSignal[i] - expectedResultsMultipexResults[i] > 0.0000001f)
		{
			fprintf(stderr, "MultiplexDemultiplexUnitTest: Multiplexing is producing the wrong results!\n");
			fprintf(stderr, "Expected: %f, Actual: %f\n", expectedResultsMultipexResults[i], RFIMMemBlock->h_inputSignal[i]);
			exit(1);
		}

		//printf("signal[%lu]: %f\n", i, RFIMMemBlock->h_inputSignal[i]);
	}



	//Demultiplex the data
	WorkerThreadDeMultiplexData(RFIMMemBlock, configuration);


	//Check the results of demultiplexing
	for(uint32_t i = 0; i < totalSignalLength; ++i)
	{
		if(RFIMMemBlock->h_outputSignal[i] - i > 0.0000001f)
		{
			fprintf(stderr, "MultiplexDemultiplexUnitTest: De-multiplexing is producing the wrong results!\n");
			fprintf(stderr, "Expected: %f, Actual: %f\n", (float)i, RFIMMemBlock->h_outputSignal[i]);
		}

		//printf("signal[%lu]: %f\n", i, RFIMMemBlock->h_outputSignal[i]);
	}


	//Free all memory
	delete RFIMMemBlock;
	delete configuration;


}




//Unit test to check that calculating the mean of a signal works correctly
void MeanUnitTest()
{
	uint32_t valuesPerSample = 2;
	uint32_t sampleNum = 4;
	uint32_t batchSize = 1;

	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, sampleNum, 0, batchSize);

	//uint64_t signalLength = valuesPerSample * sampleNum * batchSize;

	/*
	//Set the host signal
	for(uint32_t i = 0; i < signalLength; ++i)
	{
		RFIMStruct->h_inputSignal[i] = i + 1;
	}
	*/

	RFIMStruct->h_inputSignal[0] = 32;
	RFIMStruct->h_inputSignal[1] = 4;
	RFIMStruct->h_inputSignal[2] = 18;
	RFIMStruct->h_inputSignal[3] = 13;
	RFIMStruct->h_inputSignal[4] = 93;
	RFIMStruct->h_inputSignal[5] = 19;
	RFIMStruct->h_inputSignal[6] = 28;
	RFIMStruct->h_inputSignal[7] = 2;


	//Compute the mean vector and mean outer product matrix
	CalculateMeanMatrices(RFIMStruct);




	//print the results

	//Mean vector
	for(uint64_t i = 0; i < valuesPerSample; ++i)
	{
		printf("MeanVec[%llu]: %f\n", i, RFIMStruct->h_meanVec[i]);
	}


	uint64_t meanMatrixLength = valuesPerSample * valuesPerSample * batchSize;
	for(uint64_t i = 0; i < meanMatrixLength; ++i)
	{
		printf("MeanMatrix[%llu]: %f\n", i, RFIMStruct->h_covarianceMatrix[i]);
	}



	delete RFIMStruct;


	/*
	MeanVec[0]: 6.250000
	MeanVec[1]: 8.750000
	MeanVec[2]: 11.250000
	MeanVec[3]: 8.750000
	MeanVec[4]: 12.250000
	MeanVec[5]: 15.750000
	MeanVec[6]: 11.250000
	MeanVec[7]: 15.750000
	MeanVec[8]: 20.250000
	MeanVec[9]: 72.250000
	MeanVec[10]: 80.750000
	MeanVec[11]: 89.250000
	MeanVec[12]: 80.750000
	MeanVec[13]: 90.250000
	MeanVec[14]: 99.750000
	MeanVec[15]: 89.250000
	MeanVec[16]: 99.750000
	MeanVec[17]: 110.250000
	MeanVec[18]: 210.250000
	MeanVec[19]: 224.750000
	MeanVec[20]: 239.250000
	MeanVec[21]: 224.750000
	MeanVec[22]: 240.250000
	MeanVec[23]: 255.750000
	MeanVec[24]: 239.250000
	MeanVec[25]: 255.750000
	MeanVec[26]: 272.250000
	MeanVec[27]: 420.250000
	MeanVec[28]: 440.750000
	MeanVec[29]: 461.250000
	MeanVec[30]: 440.750000
	MeanVec[31]: 462.250000
	MeanVec[32]: 483.750000
	MeanVec[33]: 461.250000
	MeanVec[34]: 483.750000
	MeanVec[35]: 506.250000
	MeanVec[36]: 702.250000
	MeanVec[37]: 728.750000
	MeanVec[38]: 755.250000
	MeanVec[39]: 728.750000
	MeanVec[40]: 756.250000
	MeanVec[41]: 783.750000
	MeanVec[42]: 755.250000
	MeanVec[43]: 783.750000
	MeanVec[44]: 812.250000
	*/
}


void CovarianceMatrixUnitTest()
{
	uint32_t valuesPerSample = 2;
	uint32_t sampleNum = 4;
	uint32_t batchSize = 1;

	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, sampleNum, 0, batchSize);

	uint64_t signalLength = valuesPerSample * sampleNum * batchSize;

	RFIMStruct->h_inputSignal[0] = 32;
	RFIMStruct->h_inputSignal[1] = 4;
	RFIMStruct->h_inputSignal[2] = 18;
	RFIMStruct->h_inputSignal[3] = 13;
	RFIMStruct->h_inputSignal[4] = 93;
	RFIMStruct->h_inputSignal[5] = 19;
	RFIMStruct->h_inputSignal[6] = 28;
	RFIMStruct->h_inputSignal[7] = 2;


	/*
	//Set the host signal
	for(uint32_t i = 0; i < signalLength; ++i)
	{
		RFIMStruct->h_inputSignal[i] = i + 1;
	}
	*/


	//calculate the covariance matrix
	CalculateCovarianceMatrix(RFIMStruct);


	uint64_t covarianceMatrixLength = valuesPerSample * valuesPerSample * batchSize;

	for(uint32_t i = 0; i < covarianceMatrixLength; ++i)
	{
		/*
		if(fabs(RFIMStruct->h_covarianceMatrix[i] - 2.25f) > 0.0000001 )
		{
			fprintf(stderr, "CovarianceMatrixUnitTest: Covariance matrix is not being computed correctly!\n");
			fprintf(stderr, "Expected: %f, Actual: %f\n", 2.25f, RFIMStruct->h_covarianceMatrix[i]);
		}
		*/
	}


	//print the result
	for(uint64_t i = 0; i < covarianceMatrixLength; ++i)
	{
		printf("CovarianceMatrixHost[%llu]: %f\n", i, RFIMStruct->h_covarianceMatrix[i]);
	}


	delete RFIMStruct;
}



void EigenvectorSolverUnitTest()
{

	uint64_t valuesPerSample = 2;
	uint64_t numberOfSamples = 4;
	uint64_t batchSize = 1;


	uint64_t singleCovarianceMatrixLength = valuesPerSample * valuesPerSample;
	//uint64_t covarianceMatrixLength = singleCovarianceMatrixLength * batchSize;
	//uint64_t covarianceMatrixByteSize = sizeof(float) * covarianceMatrixLength;


	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, numberOfSamples, 0, batchSize);


	RFIMStruct->h_inputSignal[0] = 32;
	RFIMStruct->h_inputSignal[1] = 4;
	RFIMStruct->h_inputSignal[2] = 18;
	RFIMStruct->h_inputSignal[3] = 13;
	RFIMStruct->h_inputSignal[4] = 93;
	RFIMStruct->h_inputSignal[5] = 19;
	RFIMStruct->h_inputSignal[6] = 28;
	RFIMStruct->h_inputSignal[7] = 2;

	CalculateCovarianceMatrix(RFIMStruct);

	for(uint64_t i = 0; i < singleCovarianceMatrixLength * batchSize; ++i)
	{
		printf("EigenvectorSolverUnitTest[%llu]: %f\n", i, RFIMStruct->h_covarianceMatrix[i]);
	}


	//Compute the eigenvectors/values
	EigenvalueSolver(RFIMStruct);



	//Check against expected results
	float eigenvalueExpectedResults[2];
	eigenvalueExpectedResults[0] = 23.978f;
	eigenvalueExpectedResults[1] = 890.960f;

	float eigenvectorExpectedResults[4];
	eigenvectorExpectedResults[0] = -0.164f;
	eigenvectorExpectedResults[1] = 0.986f;
	eigenvectorExpectedResults[2] = 0.986f;
	eigenvectorExpectedResults[3] = 0.164f;



	//Check the results
	//Eigenvalues
	for(uint64_t i = 0; i < batchSize; ++i)
	{
		float* currentS = RFIMStruct->h_S + (i * valuesPerSample);

		bool failed = false;

		if(fabs(currentS[0]) - fabs(eigenvalueExpectedResults[0]) > 0.000001f)
		{
			failed = true;
		}

		if(fabs(currentS[1]) - fabs(eigenvalueExpectedResults[1]) > 0.000001f)
		{
			failed = true;
		}


		for(uint64_t j = 0; j < 2; ++j)
		{
			printf("Eigenvalue[%llu][%llu] = %f\n", i, j, currentS[j]);
		}




		if(failed)
		{
			fprintf(stderr, "EigendecompProduction unit test: eigenvalues are not being computed properly!\n");
			//exit(1);
		}

	}


	//Check eigenvectors
	for(uint64_t i = 0; i < batchSize; ++i)
	{

		//At this point the covariance matrix contains the eigenvectors
		float* currentU = RFIMStruct->h_covarianceMatrix + (i * valuesPerSample * valuesPerSample);

		bool failed = false;

		if(fabs( fabs(currentU[0]) - fabs(eigenvectorExpectedResults[0]) ) > 0.000001f)
		{
			failed = true;
		}
		if(fabs( fabs(currentU[1]) - fabs(eigenvectorExpectedResults[1]) ) > 0.000001f)
		{
			failed = true;
		}
		if(fabs( fabs(currentU[2]) - fabs(eigenvectorExpectedResults[2]) ) > 0.000001f)
		{
			failed = true;
		}
		if(fabs( fabs(currentU[3]) - fabs(eigenvectorExpectedResults[3]) ) > 0.000001f)
		{
			failed = true;
		}


		for(uint64_t j = 0; j < 4; ++j)
		{
			printf("Eigenvector[%llu][%llu] = %f\n", i, j, currentU[j]);
		}



		if(failed)
		{
			fprintf(stderr, "EigendecompProduction unit test: eigenvectors are not being computed properly!\n");
			//exit(1);
		}

	}


	//Free all the memory


	delete RFIMStruct;

}



void ProjectionDeprojectionUnitTest()
{
	uint64_t valuesPerSample = 2;
	uint64_t numberOfSamples = 4;
	uint64_t dimensionsToReduce = 0;
	uint64_t batchSize = 2;


	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, numberOfSamples, dimensionsToReduce, batchSize);


	uint64_t singleSignalLength = valuesPerSample * numberOfSamples;
	uint64_t signalLength = singleSignalLength * batchSize;
	uint64_t signalByteSize = sizeof(float) * signalLength;

	float* h_signalCopy = (float*)malloc(signalByteSize);


	//Set the signal
	for(uint64_t i = 0; i < batchSize; ++i)
	{
		float* currentSignal = RFIMStruct->h_inputSignal + (i * singleSignalLength);
		float* currentSignalCopy = h_signalCopy + (i * singleSignalLength);

		currentSignal[0] = 32.0f;
		currentSignalCopy[0] = currentSignal[0];

		currentSignal[1] = 4.0f;
		currentSignalCopy[1] = currentSignal[1];

		currentSignal[2] = 18.0f;
		currentSignalCopy[2] = currentSignal[2];


		currentSignal[3] = 13.0f;
		currentSignalCopy[3] = currentSignal[3];

		currentSignal[4] = 93.0f;
		currentSignalCopy[4] = currentSignal[4];

		currentSignal[5] = 19.0f;
		currentSignalCopy[5] = currentSignal[5];

		currentSignal[6] = 28.0f;
		currentSignalCopy[6] = currentSignal[6];


		currentSignal[7] = 2.0f;
		currentSignalCopy[7] = currentSignal[7];
	}



	//Calculate the covarianceMatrices
	CalculateCovarianceMatrix(RFIMStruct);


	//Calculate the eigenvectors/values
	EigenvalueSolver(RFIMStruct);



	//Allocate memory for the filtered signal
	//float* h_filteredSignal = (float*)malloc(signalByteSize);


	//Do the projection/reprojection
	EigenReductionAndFiltering(RFIMStruct);



	//print/check the results
	for(uint64_t i = 0; i < batchSize; ++i)
	{
		float* originalSignal = h_signalCopy  + (i * singleSignalLength); //Copy of the original signal
		float* currentFilteredSignal = RFIMStruct->h_inputSignal + (i * singleSignalLength); //Signal after projection/deprojection

		for(uint64_t j = 0; j < 4; ++j)
		{
			printf("originalSignal[%llu][%llu]: %f\n", i, j, originalSignal[j]);
			printf("filteredSignal[%llu][%llu]: %f\n", i, j, currentFilteredSignal[j]);

			/*
			if(originalSignal[j] - currentFilteredSignal[j] > 0.000001f)
			{
				fprintf(stderr, "FilteringProductionHost unit test: results are different from expected!\n");
				fprintf(stderr, "Expected %f, Actual: %f\n", originalSignal[j], currentFilteredSignal[j]);
				exit(1);
			}
			*/
		}
	}



	//Free all memory
	free(h_signalCopy);


	delete RFIMStruct;
}



void Remove1DProjectionDeprojectionUnitTest()
{
	uint64_t valuesPerSample = 2;
	uint64_t numberOfSamples = 4;
	uint64_t dimensionsToReduce = 1;
	uint64_t batchSize = 0;


	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, numberOfSamples, dimensionsToReduce, batchSize);


	uint64_t singleSignalLength = valuesPerSample * numberOfSamples;
	uint64_t signalLength = singleSignalLength * batchSize;
	uint64_t signalByteSize = sizeof(float) * signalLength;

	float* h_signalCopy = (float*)malloc(signalByteSize);


	//Set the signal
	for(uint64_t i = 0; i < batchSize; ++i)
	{
		float* currentSignal = RFIMStruct->h_inputSignal + (i * singleSignalLength);
		float* currentSignalCopy = h_signalCopy + (i * singleSignalLength);

		currentSignal[0] = 32.0f;
		currentSignalCopy[0] = currentSignal[0];

		currentSignal[1] = 4.0f;
		currentSignalCopy[1] = currentSignal[1];

		currentSignal[2] = 18.0f;
		currentSignalCopy[2] = currentSignal[2];


		currentSignal[3] = 13.0f;
		currentSignalCopy[3] = currentSignal[3];

		currentSignal[4] = 93.0f;
		currentSignalCopy[4] = currentSignal[4];

		currentSignal[5] = 19.0f;
		currentSignalCopy[5] = currentSignal[5];

		currentSignal[6] = 28.0f;
		currentSignalCopy[6] = currentSignal[6];


		currentSignal[7] = 2.0f;
		currentSignalCopy[7] = currentSignal[7];
	}



	//Calculate the covarianceMatrices
	CalculateCovarianceMatrix(RFIMStruct);


	//Calculate the eigenvectors/values
	EigenvalueSolver(RFIMStruct);



	//Allocate memory for the filtered signal
	//float* h_filteredSignal = (float*)malloc(signalByteSize);


	//Do the projection/reprojection
	EigenReductionAndFiltering(RFIMStruct);



	//print/check the results
	for(uint64_t i = 0; i < batchSize; ++i)
	{
		float* originalSignal = h_signalCopy  + (i * singleSignalLength); //Copy of the original signal
		float* currentFilteredSignal = RFIMStruct->h_inputSignal + (i * singleSignalLength); //Signal after projection/deprojection

		for(uint64_t j = 0; j < valuesPerSample * numberOfSamples; ++j)
		{
			printf("originalSignal[%llu][%llu]: %f\n", i, j, originalSignal[j]);
			printf("filteredSignal[%llu][%llu]: %f\n\n", i, j, currentFilteredSignal[j]);

			/*
			if(originalSignal[j] - currentFilteredSignal[j] > 0.000001f)
			{
				fprintf(stderr, "FilteringProductionHost unit test: results are different from expected!\n");
				fprintf(stderr, "Expected %f, Actual: %f\n", originalSignal[j], currentFilteredSignal[j]);
				exit(1);
			}
			*/
		}
	}



	//Free all memory
	free(h_signalCopy);


	delete RFIMStruct;
}



//Pack and unpack filterbank files without doing RFIM
//Tests to see if the unpack and pack functionality works correctly
//The input and output filterbank files should be identical
void PackingUnpackingUnitTest()
{
	uint32_t beamNum = 13;
	uint32_t rawDataBlockNum = 2;
	uint32_t numberOfWorkerThreads = 14;
	uint32_t windowSize = 15625;
	uint32_t dimensionsToReduce = 0;

	std::string inputFilenamePrefix = "/lustre/projects/p002_swin/surveys/SUPERB/2016-01-05-12:07:06/";
	std::string inputFilenamePostfix = "/2016-01-05-12:07:06.fil";

	std::string outputFilenamePrefix = "/lustre/projects/p002_swin/vvillani/PackingUnpackingUnitTest/";
	std::string outputFilenamePostfix = ".fil";

	RFIMConfiguration configuration = RFIMConfiguration(numberOfWorkerThreads, windowSize, beamNum, dimensionsToReduce, rawDataBlockNum,
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
				exit(1);
			}
		}


		//Free all allocated data
		delete originalFilterbank;
		delete outputFilterbank;

		delete [] originalFilterbankData;
		delete [] outputFilterbankData;

	}




}
