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

#include "../Header/RFIMHelperFunctions.h"
#include "../Header/RFIMMemoryBlock.h"

void RunAllUniTests()
{
	MeanUnitTest();
	CovarianceMatrixUnitTest();
	EigenvectorSolverUnitTest();
	ProjectionDeprojectionUnitTest();

	//PackingUnpackingUnitTest();

	std::cout << "All unit tests complete!" << std::endl;
}



//Unit test to check that calculating the mean of a signal works correctly
void MeanUnitTest()
{
	uint32_t valuesPerSample = 3;
	uint32_t sampleNum = 2;
	uint32_t batchSize = 5;

	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, sampleNum, 0, batchSize);

	uint64_t signalLength = valuesPerSample * sampleNum * batchSize;

	//Set the host signal
	for(uint32_t i = 0; i < signalLength; ++i)
	{
		RFIMStruct->h_inputSignal[i] = i + 1;
	}


	//Compute the mean vector and mean outer product matrix
	CalculateMeanMatrices(RFIMStruct);

	/*
	uint64_t meanMatrixLength = valuesPerSample * valuesPerSample * batchSize;

	//print the results

	for(uint64_t i = 0; i < meanMatrixLength; ++i)
	{
		printf("MeanVec[%llu]: %f\n", i, RFIMStruct->h_covarianceMatrix[i]);
	}
	*/


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
	uint32_t valuesPerSample = 3;
	uint32_t sampleNum = 2;
	uint32_t batchSize = 5;

	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, sampleNum, 0, batchSize);

	uint64_t signalLength = valuesPerSample * sampleNum * batchSize;

	//Set the host signal
	for(uint32_t i = 0; i < signalLength; ++i)
	{
		RFIMStruct->h_inputSignal[i] = i + 1;
	}


	//calculate the covariance matrix
	CalculateCovarianceMatrix(RFIMStruct);


	uint64_t covarianceMatrixLength = valuesPerSample * valuesPerSample * batchSize;

	for(uint32_t i = 0; i < covarianceMatrixLength; ++i)
	{
		if(fabs(RFIMStruct->h_covarianceMatrix[i] - 2.25f) > 0.0000001 )
		{
			fprintf(stderr, "CovarianceMatrixUnitTest: Covariance matrix is not being computed correctly!\n");
			fprintf(stderr, "Expected: %f, Actual: %f\n", 2.25f, RFIMStruct->h_covarianceMatrix[i]);
		}
	}

	/*
	//print the result
	for(uint64_t i = 0; i < covarianceMatrixLength; ++i)
	{
		printf("CovarianceMatrixHost[%llu]: %f\n", i, RFIMStruct->h_covarianceMatrix[i]);
	}
	*/

	delete RFIMStruct;
}



void EigenvectorSolverUnitTest()
{

	uint64_t valuesPerSample = 2;
	uint64_t numberOfSamples = 2;
	uint64_t batchSize = 20;


	uint64_t singleCovarianceMatrixLength = valuesPerSample * valuesPerSample;
	//uint64_t covarianceMatrixLength = singleCovarianceMatrixLength * batchSize;
	//uint64_t covarianceMatrixByteSize = sizeof(float) * covarianceMatrixLength;


	RFIMMemoryBlock* RFIMStruct = new RFIMMemoryBlock(valuesPerSample, numberOfSamples, 0, batchSize);


	//float* h_covarianceMatrices = (float*)malloc(covarianceMatrixByteSize);


	//Set the matrices
	for(uint64_t i = 0; i < batchSize; ++i)
	{
		float* currentCovarianceMatrix = RFIMStruct->h_covarianceMatrix + (i * singleCovarianceMatrixLength);

		currentCovarianceMatrix[0] = 5.0f;
		currentCovarianceMatrix[1] = 2.0f;
		currentCovarianceMatrix[2] = 2.0f;
		currentCovarianceMatrix[3] = 5.0f;

	}


	//Compute the eigenvectors/values
	EigenvalueSolver(RFIMStruct);



	//Check against expected results
	float eigenvalueExpectedResults[2];
	eigenvalueExpectedResults[0] = 3.0f;
	eigenvalueExpectedResults[1] = 7.0f;

	float eigenvectorExpectedResults[4];
	eigenvectorExpectedResults[0] = -0.707107f;
	eigenvectorExpectedResults[1] = 0.707107f;
	eigenvectorExpectedResults[2] = -0.707107f;
	eigenvectorExpectedResults[3] = -0.707107f;




	//Eigenvector[19][0] = -0.707107
	//Eigenvector[19][1] = 0.707107
	//Eigenvector[19][2] = 0.707107
	//Eigenvector[19][3] = 0.707107


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

		/*
		for(uint64_t j = 0; j < 2; ++j)
		{
			printf("Eigenvalue[%llu][%llu] = %f\n", i, j, currentS[j]);
		}
		*/


		if(failed)
		{
			fprintf(stderr, "EigendecompProduction unit test: eigenvalues are not being computed properly!\n");
			exit(1);
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

		/*
		for(uint64_t j = 0; j < 4; ++j)
		{
			printf("Eigenvector[%llu][%llu] = %f\n", i, j, currentU[j]);
		}
		*/


		if(failed)
		{
			fprintf(stderr, "EigendecompProduction unit test: eigenvectors are not being computed properly!\n");
			exit(1);
		}
	}


	//Free all the memory


	delete RFIMStruct;

}



void ProjectionDeprojectionUnitTest()
{
	uint64_t valuesPerSample = 2;
	uint64_t numberOfSamples = 3; //THIS MAY MAKE THE UNIT TEST FAIL!?
	uint64_t dimensionsToReduce = 0;
	uint64_t batchSize = 20;


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

		currentSignal[0] = 1.0f;
		currentSignalCopy[0] = currentSignal[0];

		currentSignal[1] = 2.0f;
		currentSignalCopy[1] = currentSignal[1];

		currentSignal[2] = 7.0f;
		currentSignalCopy[2] = currentSignal[2];


		currentSignal[3] = -8.0f;
		currentSignalCopy[3] = currentSignal[3];
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
			printf("filteredSignal[%llu][%llu]: %f\n", i, j, currentFilteredSignal[j]);

			if(originalSignal[j] - currentFilteredSignal[j] > 0.000001f)
			{
				fprintf(stderr, "FilteringProductionHost unit test: results are different from expected!\n");
				fprintf(stderr, "Expected %f, Actual: %f\n", originalSignal[j], currentFilteredSignal[j]);
				exit(1);
			}
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
	uint32_t rawDataBlockNum = 10;
	uint32_t numberOfWorkerThreads = 8;
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
