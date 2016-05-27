/*
 * RFIMHelperFunctions.cpp
 *
 *  Created on: 13 May 2016
 *      Author: vincentvillani
 */

#include "../Header/RFIMHelperFunctions.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>

#ifdef BUILD_WITH_MKL
#include <mkl.h>
#include <mkl_lapacke.h>
#endif


float CalculateMean(float* dataArray, uint64_t dataLength)
{
#ifdef BUILD_WITH_MKL

	float result = cblas_sasum(dataLength, dataArray, 1);

	return result / dataLength;
#else
	float result = 0;

	for(uint64_t i = 0; i < dataLength; ++i)
	{
		result += dataArray[i];
	}

	return result / dataLength;
#endif

}


float CalculateStandardDeviation(float* dataArray, uint64_t dataLength)
{

	float mean = CalculateMean(dataArray, dataLength);

	float result = 0;

	for(uint64_t i = 0; i < dataLength; ++i)
	{
		//Square the difference and sum it
		result += powf((dataArray[i] - mean), 2.0f);
	}

	return sqrtf(result / dataLength);
}




float CalculateStandardDeviation(float* dataArray, uint64_t dataLength, float mean)
{
	float result = 0;

	for(uint64_t i = 0; i < dataLength; ++i)
	{
		//Square the difference and sum it
		result += powf((dataArray[i] - mean), 2.0f);
	}

	return sqrtf(result / dataLength);
}



void CalculateMeanMatrices(RFIMMemoryBlock* rfimMemBlock)
{

#ifdef BUILD_WITH_MKL

	float alphaOne = 1.0f / rfimMemBlock->h_numberOfSamples;
	float alphaTwo = 1;
	float beta = 0;

	uint64_t signalMatrixOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_numberOfSamples;
	uint64_t meanVecOffset = rfimMemBlock->h_valuesPerSample;
	uint64_t covarianceMatrixOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_valuesPerSample;


	//Compute the mean vector
	//We use the same d_onevec each time
	for(uint64_t i = 0; i < rfimMemBlock->h_batchSize; ++i)
	{

		//Calculate meanVec
		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasTrans,
				1, rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_numberOfSamples,
				alphaOne, rfimMemBlock->h_oneVec, 1,
				rfimMemBlock->h_inputSignal + (i * signalMatrixOffset), rfimMemBlock->h_valuesPerSample, beta,
				rfimMemBlock->h_meanVec + (i * meanVecOffset), 1);


		//Calculate mean outer product matrix
		cblas_sgemm(CblasColMajor, CblasTrans, CblasNoTrans,
				rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_valuesPerSample, 1,
				alphaTwo, rfimMemBlock->h_meanVec + (i * meanVecOffset), 1,
				rfimMemBlock->h_meanVec + (i * meanVecOffset), 1, beta,
				rfimMemBlock->h_covarianceMatrix + (i * covarianceMatrixOffset), rfimMemBlock->h_valuesPerSample);


	}
#endif


}


void CalculateCovarianceMatrix(RFIMMemoryBlock* rfimMemBlock)
{

#ifdef BUILD_WITH_MKL


	//Calculate the mean matrices
	CalculateMeanMatrices(rfimMemBlock);


	//Calculate the covariance matrices
	//Take the outer product of the signal with itself
	float alpha = 1.0f / rfimMemBlock->h_numberOfSamples;
	float beta = -1;

	uint64_t signalOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_numberOfSamples;
	uint64_t covarianceMatrixOffset = rfimMemBlock->h_valuesPerSample * rfimMemBlock->h_valuesPerSample;




	for(uint64_t i = 0; i < rfimMemBlock->h_batchSize; ++i)
	{

		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasTrans,
				rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_valuesPerSample, rfimMemBlock->h_numberOfSamples,
				alpha, rfimMemBlock->h_inputSignal + (i * signalOffset), rfimMemBlock->h_valuesPerSample,
				rfimMemBlock->h_inputSignal + (i * signalOffset), rfimMemBlock->h_valuesPerSample, beta,
				rfimMemBlock->h_covarianceMatrix + (i * covarianceMatrixOffset), rfimMemBlock->h_valuesPerSample);

	}


#endif

}


void EigenvalueSolver(RFIMMemoryBlock* RFIMStruct)
{


#ifdef BUILD_WITH_MKL

	//SYEV
	//Compute the SVD for each covariance matrix
	//EIGENVALUES AND VECTORS ARE FROM LOWEST TO HIGHEST
	//EIGENVALUES ARE PLACED IN RFIMStruct->h_S
	//EIGENVECTORS ARE PLACED IN RFIMStruct->h_covarianceMatrix
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{

		int info = LAPACKE_ssyev(LAPACK_COL_MAJOR, 'V', 'U', RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset),
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset));


		//Check to see if everything went ok
		if(info != 0)
		{
			//If info = -i, the i-th parameter had an illegal value
			//If info = i, then sgesdd did not converge, updataing process failed
			fprintf(stderr, "Host_EigenvalueSolver: SVD computation didn't converge. Info: %d\n", info);
			exit(1);
		}
	}


	/*
	//SYEVR
	lapack_int info;
	//const char* sChar = 'S';
	//const char* charArg = (const char*)malloc(sizeof(const char));
	const char* test = "S";
	float abstol = slamch(test);
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{
		LAPACKE_ssyevr(LAPACK_COL_MAJOR, 'V', 'A', 'U',
				RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset),
				RFIMStruct->h_valuesPerSample,
				0, 0, 0, 0, abstol, &info, RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset),
				RFIMStruct->h_U + (i * RFIMStruct->h_UBatchOffset),
				RFIMStruct->h_valuesPerSample,
				(lapack_int*)RFIMStruct->h_VT);
		if(info != RFIMStruct->h_valuesPerSample)
		{
			//If info = -i, the i-th parameter had an illegal value
			//If info = i, then sgesdd did not converge, updataing process failed
			fprintf(stderr, "Host_EigenvalueSolver: Eigen computation didn't converge. Info: %d\n", info);
			exit(1);
		}
	}
	*/

	//SVD
	/*
	int info;
	//Compute the SVD for each covariance matrix
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{
		info =  LAPACKE_sgesdd(LAPACK_COL_MAJOR, 'A',
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset),
				RFIMStruct->h_U + (i * RFIMStruct->h_UBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_VT + (i * RFIMStruct->h_VTBatchOffset), RFIMStruct->h_valuesPerSample);
		//Check to see if everything went ok
		if(info != 0)
		{
			//If info = -i, the i-th parameter had an illegal value
			//If info = i, then sgesdd did not converge, updataing process failed
			fprintf(stderr, "Host_EigenvalueSolver: SVD computation didn't converge. Info: %d\n", info);
			exit(1);
		}
		/*
		//Tell the device to solve the eigenvectors
		cusolverStatus = cusolverDnSgesvd(*RFIMStruct->cusolverHandle, 'A', 'A',
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_S + (i * RFIMStruct->h_SBatchOffset),
				RFIMStruct->d_U + (i * RFIMStruct->h_UBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_VT + (i * RFIMStruct->h_VTBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_eigenWorkingSpace + (i * RFIMStruct->h_eigenWorkingSpaceBatchOffset),
				RFIMStruct->h_singleEigWorkingSpaceByteSize,
				NULL,
				RFIMStruct->d_devInfo + (i * RFIMStruct->h_devInfoBatchOffset));
	}
	*/

#endif

}


void EigenReductionAndFiltering(RFIMMemoryBlock* RFIMStruct)
{

#ifdef BUILD_WITH_MKL


	float alpha = 1;
	float beta = 0;




	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{

		//1. Put all the signals into the eigenvector space. E(t)S
		cblas_sgemm(CblasColMajor, CblasTrans, CblasNoTrans,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				alpha, RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset), RFIMStruct->h_valuesPerSample, beta,
				RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset), RFIMStruct->h_valuesPerSample);


		//2. Scale the dimensions to reduce axis of the space

		//Reset the state of the scale matrix
		//There is only one scale matrix
		memset(RFIMStruct->h_scaleMatrix, 0, sizeof(float) * RFIMStruct->h_scaleMatrixLength);

		//Set it to the identity matrix
		for(uint64_t j = 0; j < RFIMStruct->h_scaleMatrixLength; j += (RFIMStruct->h_valuesPerSample + 1))
		{
			RFIMStruct->h_scaleMatrix[j] = 1;
 		}


		//Calculate the scale factors

		//Find the average of the eigenvalues that we are not going to scale
		float eigenvalueAverage = 0;
		for(uint64_t j = 0; j < RFIMStruct->h_valuesPerSample - RFIMStruct->h_eigenVectorDimensionsToReduce; ++j)
		{
			eigenvalueAverage += RFIMStruct->h_S[ (i * RFIMStruct->h_SBatchOffset) + j ];
		}

		eigenvalueAverage = eigenvalueAverage / (RFIMStruct->h_valuesPerSample - RFIMStruct->h_eigenVectorDimensionsToReduce);

		//printf("eigenvalueAverage: %f\n", eigenvalueAverage);
		//printf("highestEigenvalue: %f\n", RFIMStruct->h_S[(i * RFIMStruct->h_SBatchOffset) + RFIMStruct->h_valuesPerSample - 1]);


		//Set the scale factors in the scale matrix
		for(uint64_t j = (RFIMStruct->h_valuesPerSample - RFIMStruct->h_eigenVectorDimensionsToReduce);
				j < RFIMStruct->h_valuesPerSample; ++j)
		{

			//average / eigenvalue = scale factor
			RFIMStruct->h_scaleMatrix[ (j * RFIMStruct->h_valuesPerSample) + j ] = //1.0f -
					(eigenvalueAverage / RFIMStruct->h_S[(i * RFIMStruct->h_SBatchOffset) + j]);


		}






		//Do the scaling multiplication
		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				alpha, RFIMStruct->h_scaleMatrix, RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset), RFIMStruct->h_valuesPerSample, beta,
				RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset), RFIMStruct->h_valuesPerSample);



		//3. Put the signal back into the original space
		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				alpha, RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset), RFIMStruct->h_valuesPerSample, beta,
				RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset), RFIMStruct->h_valuesPerSample);


	}













#endif

}




