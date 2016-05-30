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




int compareFunction(const void* a, const void* b)
{
	float floata = *((float*)a);
	float floatb = *((float*)b);

	return floata - floatb;
}

float CalculateMedian(float* dataArray, uint64_t dataLength)
{
	//Sort the data
	qsort(dataArray, dataLength, sizeof(float), &compareFunction);

	//return the median of the sorted data
	return dataArray[ (uint64_t)(dataLength / 2.0f) ];
}


float CalculateMeanAbsoluteDeviation(float* dataArray, float* workingSpace, uint64_t dataLength)
{
	//Copy the data into the working space
	memcpy(workingSpace, dataArray, dataLength * sizeof(float));

	float median1 = CalculateMedian(workingSpace, dataLength);

	//Now get the absolute deviations from that median
	for(uint64_t i = 0; i < dataLength; ++i)
	{
		workingSpace[i] = fabs(workingSpace[i] - median1);
	}

	//Find the median of the absolute deviations
	return CalculateMedian(workingSpace, dataLength);

}



//Used to detect outliers in a dataset
//If the absolute value of a modified z-score is higher than 3.5 it can be considered a potential outlier
//http://www.itl.nist.gov/div898/handbook/eda/section3/eda35h.htm
float CalculateModifiedZScore(float sample, float median, float meanAbsoluteDeviation)
{
	return  fabs( (0.6745f * (sample - median)) / meanAbsoluteDeviation );
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


void EigenReductionAndFiltering(RFIMMemoryBlock* RFIMStruct, RFIMConfiguration* RFIMConfiguration)
{

#ifdef BUILD_WITH_MKL


	float alpha = 1;
	float beta = 0;

	uint64_t totalDimensionsRemoved = 0;


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


		//Detect outliers in the data set

		float* currentEigenvalues = RFIMStruct->h_S + (i * RFIMStruct->h_SBatchOffset);

		//The eigenvalues are already sorted, just get one in the middle or close to the middle (if even)
		//float eigenvalueMedian =  *(currentEigenvalues + (uint64_t)(RFIMStruct->h_valuesPerSample / 2.0f));

		//Calculate the median absolute deviation
		//Use the h_meanvec as working space, we don't need it at this point
		//float medianAbsoluteDeviation = CalculateMeanAbsoluteDeviation(currentEigenvalues,
		//		RFIMStruct->h_meanVec, RFIMStruct->h_valuesPerSample);


		uint32_t eigenvaluesToRemove = 0;
		float eigenvalueAverage = 0;

		//float powerThreshold = 2.0f;

		//Detect outliers in the dataset using modified z-values
		//A value over 3.5f can be considered an outlier
		//If it's not over 3.5, add it to the average
		for(uint64_t j = 0; j < RFIMStruct->h_valuesPerSample; ++j)
		{
			//float modifiedZValue = CalculateModifiedZScore(currentEigenvalues[j], eigenvalueMedian, medianAbsoluteDeviation);

			//if the difference in magnitude of the eigenvalues is bigger than the threshold, remove it?
			//float eigenValueDifference = currentEigenvalues[j + 1] - currentEigenvalues[j];

			if( currentEigenvalues[j] > RFIMConfiguration->powerThreshold ) //eigenValueDifference > threshold && eigenValueDifference - (eigenvalueAverage / j) > threshold )
			{
				eigenvaluesToRemove += 1;
			}
			else
			{
				eigenvalueAverage += currentEigenvalues[j];
			}
		}

		//Average the eigenvalue power by the number of dimensions we are NOT going to remove
		eigenvalueAverage /= (RFIMStruct->h_valuesPerSample - eigenvaluesToRemove);

		//There is no RFI here, skip the next step
		if(eigenvaluesToRemove == 0)
		{
			//TODO: Line it up somehow so I don't have to do this?
			//Just copy the signal across so the next step has valid data
			memcpy(RFIMStruct->h_outputSignal + (i * RFIMStruct->h_outputSignalBatchOffset),
					RFIMStruct->h_inputSignal + (i * RFIMStruct->h_inputSignalBatchOffset),
					sizeof(float) * RFIMStruct->h_valuesPerSample * RFIMStruct->h_numberOfSamples);

			//Set the mask value for the freq channel, if we are generating one
			if(RFIMStruct->h_generatingMask)
			{
				RFIMStruct->h_maskValues[i] = 0;
			}

			continue;
		}


		//Set the mask value for the freq channel, if we are generating one
		//Set this byte for the mask
		if(RFIMStruct->h_generatingMask)
		{
			RFIMStruct->h_maskValues[i] = 1;
		}

		//Keep track of how many dimensions are removed
		totalDimensionsRemoved += eigenvaluesToRemove;


		//Set the scale factors in the scale matrix
		for(uint64_t j = (RFIMStruct->h_valuesPerSample - eigenvaluesToRemove);
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



	//if(totalDimensionsRemoved > 0)
	//	printf("Dimensions removed: %llu\n", totalDimensionsRemoved);




#endif

}




