/*
 * RFIMMemoryBlock.cpp
 *
 *  Created on: 7 May 2016
 *      Author: vincent
 */

#include "../Header/RFIMMemoryBlock.h"

#include <stdio.h>


	RFIMMemoryBlock::RFIMMemoryBlock(uint64_t h_valuesPerSample, uint64_t h_numberOfSamples, uint64_t h_dimensionToReduce, uint64_t h_batchSize)
	{


		//Set signal attributes
		//------------------------
		this->h_valuesPerSample = h_valuesPerSample;
		this->h_numberOfSamples = h_numberOfSamples;
		this->h_eigenVectorDimensionsToReduce = h_dimensionToReduce;
		this->h_batchSize = h_batchSize;




		//Setup the one vec, we use the same memory over and over again, it should never change
		//------------------------

		//Setup space for the input signal


		uint64_t inputSignalSingleLength = h_valuesPerSample * h_numberOfSamples;
		uint64_t inputSignalLength = inputSignalSingleLength * h_batchSize;

		this->h_inputSignal = new (std::nothrow) float[inputSignalLength];
		this->h_inputSignalBatchOffset = inputSignalSingleLength;

		if(this->h_inputSignal == NULL)
		{
			fprintf(stderr, "RFIMMemoryBlock::RFIMMemoryBlock(): Error allocating memory %llu bytes for h_inputSignal\n",
					sizeof(float) * inputSignalLength);
			exit(1);
		}




		uint64_t oneVecLength = h_numberOfSamples;
		//uint64_t oneVecByteSize = sizeof(float) * oneVecLength;

		this->h_oneVec = new (std::nothrow) float[oneVecLength];

		if(this->h_oneVec == NULL)
		{
			fprintf(stderr, "RFIMMemoryBlock::RFIMMemoryBlock(): Error allocating memory %llu bytes for h_oneVec\n",
								sizeof(float) * oneVecLength);
			exit(1);
		}

		//Fill the one vec with ones
		for(uint64_t i = 0; i < oneVecLength; ++i)
		{
			this->h_oneVec[i] = 1;
		}


		//Mean vec
		uint64_t meanVecLength = h_valuesPerSample * h_batchSize;
		//uint64_t meanVecByteSize = sizeof(float) * meanVecLength;

		this->h_meanVec = new (std::nothrow) float[meanVecLength];
		this->h_meanVecBatchOffset = h_valuesPerSample;

		if(this->h_meanVec == NULL)
		{
			fprintf(stderr, "RFIMMemoryBlock::RFIMMemoryBlock(): Error allocating memory %llu bytes for h_meanVec\n",
								sizeof(float) * meanVecLength);
			exit(1);
		}



		//Setup the covariance matrix
		//------------------------
		uint64_t covarianceMatrixLength = h_valuesPerSample * h_valuesPerSample * h_batchSize;
		//uint64_t covarianceMatrixByteSize = sizeof(float) * covarianceMatrixLength;
		this->h_covarianceMatrixBatchOffset = h_valuesPerSample * h_valuesPerSample;

		this->h_covarianceMatrix = new (std::nothrow) float[covarianceMatrixLength]; //(float*)malloc(covarianceMatrixByteSize);

		if(this->h_covarianceMatrix == NULL)
		{
			fprintf(stderr, "RFIMMemoryBlock::RFIMMemoryBlock(): Error allocating memory %llu bytes for h_covarianceMatrix\n",
								sizeof(float) * covarianceMatrixLength);
			exit(1);
		}

		/*
		//Eigenvector stuff
		//------------------------
		uint64_t singleULength = h_valuesPerSample * h_valuesPerSample;
		uint64_t ULength = singleULength * h_batchSize;
		//uint64_t UByteSize = sizeof(float) * ULength;

		this->h_U = new float[ULength]; //(float*)malloc(UByteSize);
		this->h_VT = new float[ULength]; //(float*)malloc(UByteSize); //VT is the same size as U

		this->h_UBatchOffset = singleULength;
		this->h_VTBatchOffset = singleULength;
		*/

		//S
		uint64_t singleSLength = h_valuesPerSample;
		uint64_t SLength = h_valuesPerSample * h_batchSize;
		//uint64_t SByteLength = sizeof(float) * SLength;

		this->h_S = new (std::nothrow) float[SLength]; //(float*)malloc(SByteLength);
		this->h_SBatchOffset = singleSLength;

		if(this->h_S == NULL)
		{
			fprintf(stderr, "RFIMMemoryBlock::RFIMMemoryBlock(): Error allocating memory %llu bytes for h_S\n",
								sizeof(float) * SLength);
			exit(1);
		}


		//Allocate memory for the column swapper matrix
		//this->h_eigenVectorColumnSwapperMatrixLength = h_valuesPerSample * h_valuesPerSample;
		//this->h_eigenvectorColumnSwapperMatrix = new float[this->h_eigenVectorColumnSwapperMatrixLength];




		//Projected signal
		//------------------------
		uint64_t projectedSignalSingleLength = h_valuesPerSample * h_numberOfSamples;
		uint64_t projectedSignalLength = projectedSignalSingleLength * h_batchSize;
		//uint64_t projectedSignalByteSize = sizeof(float) * projectedSignalLength;
		this->h_outputSignal = new (std::nothrow) float[projectedSignalLength];

		this->h_outputSignalBatchOffset = projectedSignalSingleLength;

		if(this->h_outputSignal == NULL)
		{
			fprintf(stderr, "RFIMMemoryBlock::RFIMMemoryBlock(): Error allocating memory %llu bytes for h_outputSignal\n",
								sizeof(float) * projectedSignalLength);
			exit(1);
		}

	}


	RFIMMemoryBlock::~RFIMMemoryBlock()
	{
		delete [] this->h_inputSignal;

		delete [] this->h_oneVec;
		delete [] this->h_meanVec;

		delete [] this->h_covarianceMatrix;

		//delete [] this->h_U;
		//delete [] this->h_VT;
		delete [] this->h_S;

		//delete [] this->h_eigenvectorColumnSwapperMatrix;

		delete [] this->h_outputSignal;

	}


