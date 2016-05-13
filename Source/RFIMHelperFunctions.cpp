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

#ifdef BUILD_WITH_MKL
#include <mkl.h>
#include <mkl_lapacke.h>
#endif

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


	//Switch the columns, so that its highest...lowest eigenvectors



	//Set the appropriate number of columns to zero
	uint64_t eigenvectorZeroByteSize = sizeof(float) * RFIMStruct->h_valuesPerSample * RFIMStruct->h_eigenVectorDimensionsToReduce;

	//std::cout << "EigenZeroByteSize: " << eigenvectorZeroByteSize << std::endl;

	//Eigenvectors are stored in the covariance matrix when using MKL
	//Eigenvectors with the lowest eigenvalues are stored first
	//smallest -> largest
	float* startingEigenVector = RFIMStruct->h_covarianceMatrix + ((RFIMStruct->h_valuesPerSample - RFIMStruct->h_eigenVectorDimensionsToReduce) * RFIMStruct->h_valuesPerSample);
	uint32_t eigenVectorBatchOffset = RFIMStruct->h_valuesPerSample * RFIMStruct->h_valuesPerSample;

	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{
		memset(startingEigenVector + (i * eigenVectorBatchOffset), 0, eigenvectorZeroByteSize);

	}





	float alpha = 1;
	float beta = 0;

	uint64_t originalSignalBatchOffset = RFIMStruct->h_valuesPerSample * RFIMStruct->h_numberOfSamples;


	//Do the projection
	for(uint64_t i = 0; i < RFIMStruct->h_batchSize; ++i)
	{


		//Projected signal matrix
		//Ps = (Er Transposed) * Os
		cblas_sgemm(CblasColMajor, CblasTrans, CblasNoTrans,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				alpha, RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset), RFIMStruct->h_valuesPerSample, beta,
				RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset), RFIMStruct->h_valuesPerSample);


		/*
		cublasStatus = cublasSgemm_v2(*RFIMStruct->cublasHandle, CUBLAS_OP_T, CUBLAS_OP_N,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				&alpha, RFIMStruct->d_U + (i * RFIMStruct->h_UBatchOffset), RFIMStruct->h_valuesPerSample,
				d_originalSignalMatrices + (i * originalSignalBatchOffset), RFIMStruct->h_valuesPerSample, &beta,
				RFIMStruct->d_projectedSignalMatrix + (i * RFIMStruct->h_projectedSignalBatchOffset), RFIMStruct->h_valuesPerSample);

		*/


		//Do the reprojection back
		//final signal matrix
		// Fs = Er * Ps

		cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				alpha,  RFIMStruct->h_covarianceMatrix + (i * RFIMStruct->h_covarianceMatrixBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset), RFIMStruct->h_valuesPerSample, beta,
				RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset), RFIMStruct->h_valuesPerSample);



		/*
		cublasStatus_t = cublasSgemm_v2(*RFIMStruct->cublasHandle, CUBLAS_OP_N, CUBLAS_OP_N,
				RFIMStruct->h_valuesPerSample, RFIMStruct->h_numberOfSamples, RFIMStruct->h_valuesPerSample,
				&alpha, RFIMStruct->d_U + (i * RFIMStruct->h_UBatchOffset), RFIMStruct->h_valuesPerSample,
				RFIMStruct->d_projectedSignalMatrix + (i * RFIMStruct->h_projectedSignalBatchOffset), RFIMStruct->h_valuesPerSample, &beta,
				d_filteredSignals + (i * originalSignalBatchOffset), RFIMStruct->h_valuesPerSample);

		*/
	}

#endif


}

