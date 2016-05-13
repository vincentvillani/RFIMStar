/*
 * RFIMMemoryBlock.cpp
 *
 *  Created on: 7 May 2016
 *      Author: vincent
 */

#include "../Header/RFIMMemoryBlock.h"


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

		this->h_inputSignal = new float[inputSignalLength];
		this->h_inputSignalBatchOffset = inputSignalSingleLength;


		uint64_t oneVecLength = h_numberOfSamples;
		//uint64_t oneVecByteSize = sizeof(float) * oneVecLength;

		this->h_oneVec = new float[oneVecLength];  //(float*)malloc(oneVecByteSize);

		//Fill the one vec with ones
		for(uint64_t i = 0; i < oneVecLength; ++i)
		{
			this->h_oneVec[i] = 1;
		}


		//Mean vec
		uint64_t meanVecLength = h_valuesPerSample * h_batchSize;
		//uint64_t meanVecByteSize = sizeof(float) * meanVecLength;

		this->h_meanVec = new float[meanVecLength]; //(float*)malloc(meanVecByteSize);
		this->h_meanVecBatchOffset = h_valuesPerSample;



		//Setup the covariance matrix
		//------------------------
		uint64_t covarianceMatrixLength = h_valuesPerSample * h_valuesPerSample * h_batchSize;
		//uint64_t covarianceMatrixByteSize = sizeof(float) * covarianceMatrixLength;
		this->h_covarianceMatrixBatchOffset = h_valuesPerSample * h_valuesPerSample;

		this->h_covarianceMatrix = new float[covarianceMatrixLength]; //(float*)malloc(covarianceMatrixByteSize);


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

		this->h_S = new float[SLength]; //(float*)malloc(SByteLength);
		this->h_SBatchOffset = singleSLength;


		//Projected signal
		//------------------------
		uint64_t projectedSignalSingleLength = h_valuesPerSample * h_numberOfSamples;
		uint64_t projectedSignalLength = projectedSignalSingleLength * h_batchSize;
		//uint64_t projectedSignalByteSize = sizeof(float) * projectedSignalLength;
		this->h_outputSignal = new float[projectedSignalLength];

		this->h_outputSignalBatchOffset = projectedSignalSingleLength;

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

		delete [] this->h_outputSignal;

	}


